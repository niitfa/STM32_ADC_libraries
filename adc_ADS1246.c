/*
 * adc_ADS1246.c
 *
 *  Created on: Apr 2, 2025
 *      Author: Kirill
 */

#include "adc_ADS1246.h"
#include <malloc.h>
#include <string.h>
#include <math.h>

static uint8_t CMD_WAKEUP 	= 0x00;
static uint8_t CMD_SLEEP 	= 0x02;
static uint8_t CMD_SYNC 	= 0x04;
static uint8_t CMD_RESET 	= 0x04;
static uint8_t CMD_NOP 		= 0xFF;
static uint8_t CMD_RDATA 	= 0x12;
static uint8_t CMD_RDATAC 	= 0x14;
static uint8_t CMD_RREG 	= 0x20;
static uint8_t CMD_WREG 	= 0x40;
static uint8_t CMD_SYSOCAL 	= 0x60;
static uint8_t CMD_SYSGCAL 	= 0x61;
static uint8_t CMD_SELFOCAL = 0x62;

static uint8_t REG_SYS0 = 0x03;


typedef enum
{
	ADS1246_WAIT,
	ADS1246_WAKEUP,
	ADS1246_SETUP_SYS0,
	ADS1246_MEASURE
} ADS1246_state_t;

#define ADC_ADS1246_SPI_TIMEOUT 10

static void spi_select(adc_t* self);
static void spi_deselect(adc_t* self);
static void spi_hw_command(adc_t *self, uint8_t cmd);

struct adc_data_t
{
	SPI_HandleTypeDef* hspi;

	GPIO_TypeDef* portCS;
	uint16_t pinCS;
	GPIO_TypeDef* portDRDY;
	uint16_t pinDRDY;
	GPIO_TypeDef* portSTART;
	uint16_t pinSTART;
	GPIO_TypeDef* portRESET;
	uint16_t pinRESET;

	uint8_t bitResolution;

	double Vref;
	int32_t lastOutputValue;
	int32_t maxOutputValue;

	//uint8_t FR_word;
	uint8_t SYS0_conf;

	uint32_t waitCycles;
	uint32_t setupWaitCycles, setupWaitCyclesMax;

	ADS1246_state_t state;
};

static void init(adc_t* self)
{
	spi_deselect(self);
}

static void update(adc_t* self, void* option)
{
	switch(self->data->state)
	{
		case ADS1246_WAIT:
			spi_deselect(self);
			self->data->waitCycles--;
			if(self->data->portSTART)
			{
				HAL_GPIO_WritePin(self->data->portSTART, self->data->pinSTART, 1);
			}

			if(self->data->portRESET)
			{
				HAL_GPIO_WritePin(self->data->portRESET, self->data->pinRESET, 1);
			}


			if(!self->data->waitCycles)
			{
				self->data->state = ADS1246_WAKEUP;
			}
			break;
		case ADS1246_WAKEUP:
			spi_select(self);
			spi_hw_command(self, CMD_WAKEUP);
			spi_deselect(self);
			self->data->state = ADS1246_SETUP_SYS0;
			break;
		case ADS1246_SETUP_SYS0:
			spi_select(self);
			spi_hw_command(self, CMD_WREG | REG_SYS0);
			spi_hw_command(self, 1); // 1 byte
			spi_hw_command(self, self->data->SYS0_conf);
			spi_deselect(self);
			self->data->state = ADS1246_MEASURE;
			break;
		case ADS1246_MEASURE:
			const uint8_t kDataSizeBytes = 3;
			const uint8_t kBufferSizeBytes = 4;
			uint8_t rxBytes [kBufferSizeBytes];
			memset(rxBytes, 0, kBufferSizeBytes);
			spi_select(self);
			spi_hw_command(self, CMD_RDATA);
			int i;
			for(i = 0; i < kDataSizeBytes; ++i)
			{
				//HAL_SPI_Receive(self->data->hspi, rxBytes + kDataSizeBytes - i - 1, 1, ADC_ADS1246_SPI_TIMEOUT);
				HAL_SPI_TransmitReceive(self->data->hspi, &CMD_NOP, rxBytes + kDataSizeBytes - i - 1, 1, ADC_ADS1246_SPI_TIMEOUT);
				while(HAL_SPI_GetState(self->data->hspi) != HAL_SPI_STATE_READY)
					;
			}
			spi_deselect(self);
			self->data->lastOutputValue = *(int32_t*)rxBytes;
			break;
	}
}

static uint32_t get_cnt(adc_t* self)
{
	return self->data->lastOutputValue;
}

static double get_vout(adc_t* self)
{
	return self->data->Vref * get_cnt(self) / self->data->maxOutputValue;
}

static struct adc_vtable methods = {init, update, get_cnt, get_vout};

adc_t adc_ADS1246_create(
		SPI_HandleTypeDef* hspi,
		GPIO_TypeDef* portCS,
		uint16_t pinCS,
		GPIO_TypeDef* portDRDY,
		uint16_t pinDRDY,
		GPIO_TypeDef* portSTART,
		uint16_t pinSTART,
		GPIO_TypeDef* portRESET,
		uint16_t pinRESET,
		double Vref,
		uint8_t SYS0_conf,
		uint32_t waitCycles
		)
{
	adc_t adc;
	adc.vtable = &methods;
	struct adc_data_t* pdata =
			(struct adc_data_t*)malloc(sizeof(struct adc_data_t));
	if(pdata)
	{
		memset(pdata, 0, sizeof(*pdata));
		pdata->hspi = hspi;
		pdata->portCS = portCS;
		pdata->pinCS = pinCS;
		pdata->portDRDY = portDRDY;
		pdata->pinDRDY = pinDRDY;
		pdata->portSTART = portSTART;
		pdata->pinSTART = pinSTART;
		pdata->portRESET = portRESET;
		pdata->pinRESET = pinRESET;
		pdata->Vref = Vref;
		pdata->lastOutputValue = 0;
		pdata->bitResolution = 24;
		pdata->maxOutputValue = (uint32_t)pow(2, pdata->bitResolution);
		pdata->SYS0_conf = SYS0_conf;
		pdata->waitCycles = waitCycles;
		pdata->setupWaitCyclesMax = 5;
		pdata->setupWaitCycles = pdata->setupWaitCycles;
		pdata->state = ADS1246_WAIT;
		adc.data = pdata;
	}
	// init
	return adc;

}

static void spi_select(adc_t* self)
{
	HAL_GPIO_WritePin(
			self->data->portCS,
			self->data->pinCS,
			GPIO_PIN_RESET
			);
}
static void spi_deselect(adc_t* self)
{
	HAL_GPIO_WritePin(
			self->data->portCS,
			self->data->pinCS,
			GPIO_PIN_SET
			);
}

static void spi_hw_command(adc_t *self, uint8_t cmd)
{
	HAL_SPI_Transmit(self->data->hspi, &cmd, 1, ADC_ADS1246_SPI_TIMEOUT);
	while(HAL_SPI_GetState(self->data->hspi) != HAL_SPI_STATE_READY)
		;
}





