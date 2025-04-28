/*
 * adc_ADS1242.c
 *
 *  Created on: Apr 9, 2025
 *      Author: Kirill
 */

#include "adc_ADS1242.h"
#include <malloc.h>
#include <string.h>
#include <math.h>

static uint8_t CMD_RDATA 	= 0x01;
static uint8_t CMD_RDATAC 	= 0x03;
static uint8_t CMD_STOPC 	= 0x0F;
static uint8_t CMD_RREG 	= 0x10;
static uint8_t CMD_WREG 	= 0x50;
static uint8_t CMD_SELFCAL 	= 0xF0;
static uint8_t CMD_SELFOCAL = 0xF1;
static uint8_t CMD_SELFGCAL = 0xF2;
static uint8_t CMD_SYSOCAL 	= 0xF3;
static uint8_t CMD_SYSGCAL 	= 0xF4;
static uint8_t CMD_WAKEUP 	= 0xFB;
static uint8_t CMD_DSYNC 	= 0xFC;
static uint8_t CMD_SLEEP 	= 0xFD;
static uint8_t CMD_RESET 	= 0xFE;
static uint8_t CMD_NOP 		= 0xFF;

static uint8_t REG_SETUP = 0x00;
static uint8_t REG_MUX = 0x01;
static uint8_t REG_ACR = 0x02;
static uint8_t REG_ODAC = 0x03;
static uint8_t REG_DIO = 0x04;
static uint8_t REG_DIR = 0x05;
static uint8_t REG_IOCON = 0x06;
static uint8_t REG_OCR0 = 0x07;
static uint8_t REG_OCR1 = 0x08;
static uint8_t REG_OCR2 = 0x09;
static uint8_t REG_FSR0 = 0x0A;
static uint8_t REG_FSR1 = 0x0B;
static uint8_t REG_FSR2 = 0x0C;
static uint8_t REG_DOR2 = 0x0D;
static uint8_t REG_DOR1 = 0x0E;
static uint8_t REG_DOR0 = 0x0F;

typedef enum
{
	ADS1242_WAIT,
	ADS1242_WAKEUP,
	ADS1242_SEL_CH,
	ADS1242_MEASURE
} ADS1242_state_t;

#define ADC_ADS1242_SPI_TIMEOUT 10

static void spi_select(adc_t* self);
static void spi_deselect(adc_t* self);
static void spi_hw_command(adc_t *self, uint8_t cmd);
static void check_negative_24_to_32(int32_t* val);

struct adc_data_t
{
	SPI_HandleTypeDef* hspi;

	GPIO_TypeDef* portCS;
	uint16_t pinCS;
	GPIO_TypeDef* portDRDY;
	uint16_t pinDRDY;
	GPIO_TypeDef* portPDWN;
	uint16_t pinPDWN;

	uint8_t bitResolution;

	uint8_t chPositive, chNegative;

	double Vref;
	int32_t lastOutputValue;
	int32_t maxOutputValue;


	uint32_t waitCycles;
	uint32_t setupWaitCycles, setupWaitCyclesMax;

	ADS1242_state_t state;
};

static void init(adc_t* self)
{
	spi_deselect(self);
}

static void update(adc_t* self, void* option)
{
	switch(self->data->state)
	{
		case ADS1242_WAIT:
			spi_deselect(self);
			self->data->waitCycles--;
			if(self->data->portPDWN)
			{
				HAL_GPIO_WritePin(self->data->portPDWN, self->data->pinPDWN, 1);
			}
			if(!self->data->waitCycles)
			{
				self->data->state = ADS1242_WAKEUP;
			}
			break;
		case ADS1242_WAKEUP:
			self->data->state = ADS1242_SEL_CH;
			break;
		case ADS1242_SEL_CH:
			uint8_t MUX_word = (self->data->chPositive << 4) | (self->data->chNegative << 0);
			spi_select(self);
			spi_hw_command(self, CMD_WREG | REG_MUX);
			spi_hw_command(self, 0);
			spi_hw_command(self, MUX_word);
			spi_deselect(self);
			self->data->state = ADS1242_MEASURE;
			break;
		case ADS1242_MEASURE:
			const uint8_t kDataSizeBytes = 3;
			const uint8_t kBufferSizeBytes = 4;
			uint8_t rxBytes [kBufferSizeBytes];
			memset(rxBytes, 0, kBufferSizeBytes);
			spi_select(self);
			spi_hw_command(self, CMD_RDATA);
			int i;
			for(i = 0; i < kDataSizeBytes; ++i)
			{
				HAL_SPI_TransmitReceive(self->data->hspi, &CMD_NOP, rxBytes + kDataSizeBytes - i - 1, 1, ADC_ADS1242_SPI_TIMEOUT);
				while(HAL_SPI_GetState(self->data->hspi) != HAL_SPI_STATE_READY)
					;
			}
			spi_deselect(self);
			check_negative_24_to_32((int32_t*)rxBytes);
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


adc_t adc_ADS1242_create(
		SPI_HandleTypeDef* hspi,
		GPIO_TypeDef* portCS,
		uint16_t pinCS,
		GPIO_TypeDef* portDRDY,
		uint16_t pinDRDY,
		GPIO_TypeDef* portPDWN,
		uint16_t pinPDWN,
		uint8_t chPositive,
		uint8_t chNegative,
		double Vref,
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
		pdata->portPDWN = portPDWN;
		pdata->pinPDWN = pinPDWN;
		pdata->chPositive = chPositive;
		pdata->chNegative = chNegative;
		pdata->Vref = Vref;
		pdata->lastOutputValue = 0;
		pdata->bitResolution = 24;
		pdata->maxOutputValue = (uint32_t)pow(2, pdata->bitResolution);
		pdata->waitCycles = waitCycles;
		pdata->setupWaitCyclesMax = 5;
		pdata->setupWaitCycles = pdata->setupWaitCycles;
		pdata->state = ADS1242_WAIT;
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
	HAL_SPI_Transmit(self->data->hspi, &cmd, 1, ADC_ADS1242_SPI_TIMEOUT);
	while(HAL_SPI_GetState(self->data->hspi) != HAL_SPI_STATE_READY)
		;
}

static void check_negative_24_to_32(int32_t* val)
{
	if ((*val >> 23) & (int)1)
	{
		*val |= 0xFF000000;
	}

}
