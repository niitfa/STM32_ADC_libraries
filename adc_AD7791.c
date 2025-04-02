/*
 * adc_AD7791.c
 *
 *  Created on: Dec 6, 2024
 *      Author: Kirill
 */

#include "adc_AD7791.h"
#include "malloc.h"
#include <string.h>
#include <math.h>

typedef enum
{
	AD7791_WAIT,
	AD7791_SETUP_FR,
	AD7791_SETUP_MR,
	AD7791_MEASURE
} AD7791_state_t;

#define ADC_AD7791_SPI_TIMEOUT 10

static void spi_select(adc_t* self);
static void spi_deselect(adc_t* self);
static void spi_hw_command(adc_t *self, uint8_t cmd);

static void delay_us(volatile uint32_t us);

struct adc_data_t
{
	SPI_HandleTypeDef* hspi;
	GPIO_TypeDef* portCS;
	uint16_t pinCS;
	uint8_t bitResolution;
	double Vref;
	int32_t lastOutputValue;
	int32_t maxOutputValue;

	uint8_t FR_word;
	uint8_t MR_word;

	uint32_t waitCycles;
	uint32_t setupWaitCycles, setupWaitCyclesMax;

	AD7791_state_t state;
};

static void init(adc_t* self)
{
	delay_us(1);
}

static void update(adc_t* self, void* option)
{
	switch(self->data->state)
	{
	case AD7791_WAIT: // wait several cycles
		spi_deselect(self);
		self->data->waitCycles--;
		if(!self->data->waitCycles)
		{
			self->data->state = AD7791_SETUP_FR;
		}
		break;
	case AD7791_SETUP_FR: // write to filter register
		if(self->data->setupWaitCycles == self->data->setupWaitCyclesMax)
		{
			spi_select(self);
			//delay_us(1); // delay
			spi_hw_command(self, 0x20);
			spi_hw_command(self, self->data->FR_word);
			//delay_us(1); // delay
			spi_deselect(self);
		}
		if(!self->data->setupWaitCycles)
		{
			self->data->setupWaitCycles = self->data->setupWaitCyclesMax; // RESET COUNTER
			self->data->state = AD7791_SETUP_MR; // NEXT STATE
		}
		else
		{
			self->data->setupWaitCycles--;
		}
		break;
	case AD7791_SETUP_MR: // write to mode register
		if(self->data->setupWaitCycles == self->data->setupWaitCyclesMax)
		{
			spi_select(self);
			//delay_us(1); // delay
			spi_hw_command(self, 0x10);
			spi_hw_command(self, self->data->MR_word);
			//delay_us(1); // delay
			spi_deselect(self);
		}
		if(!self->data->setupWaitCycles)
		{
			self->data->setupWaitCycles = self->data->setupWaitCyclesMax; // RESET COUNTER
			self->data->state = AD7791_MEASURE;
		}
		else
		{
			self->data->setupWaitCycles--;
		}
		break;
	case AD7791_MEASURE:  // measure
		const uint8_t kDataSizeBytes = 3;
		const uint8_t kBufferSizeBytes = 4;
		uint8_t rxBytes [kBufferSizeBytes];
		memset(rxBytes, 0, kBufferSizeBytes);
		spi_select(self);
		//delay_us(1); // delay
		spi_hw_command(self, 0x38); // read data register
		int i;
		for(i = 0; i < kDataSizeBytes; ++i)
		{
			HAL_SPI_Receive(self->data->hspi, rxBytes + kDataSizeBytes - i - 1, 1, ADC_AD7791_SPI_TIMEOUT);
			while(HAL_SPI_GetState(self->data->hspi) != HAL_SPI_STATE_READY)
				;
		}
		//delay_us(1); // delay
		spi_deselect(self);
		self->data->lastOutputValue = *(int32_t*)rxBytes;

		//self->data->state = AD7791_SETUP_FR;
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

adc_t adc_AD7791_create(
		SPI_HandleTypeDef* hspi,
		GPIO_TypeDef* portCS,
		uint16_t pinCS,
		double Vref,
		uint8_t FR_word,
		uint8_t MR_word,
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
		pdata->Vref = Vref;
		pdata->lastOutputValue = 0;
		pdata->bitResolution = 24;
		pdata->maxOutputValue = (uint32_t)pow(2, pdata->bitResolution);
		pdata->FR_word = FR_word;
		pdata->MR_word = MR_word;
		pdata->waitCycles = waitCycles;
		pdata->setupWaitCyclesMax = 5;
		pdata->setupWaitCycles = pdata->setupWaitCycles;
		pdata->state = AD7791_WAIT;
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
	HAL_SPI_Transmit(self->data->hspi, &cmd, 1, ADC_AD7791_SPI_TIMEOUT);
	while(HAL_SPI_GetState(self->data->hspi) != HAL_SPI_STATE_READY)
		;
}

static void delay_us(volatile uint32_t us)
{
	int SYSTICK_LOAD = SystemCoreClock / 1000000U;
	int SYSTICK_DELAY_CALIB = SYSTICK_LOAD >> 1;
    do {
         uint32_t start = SysTick->VAL;
         uint32_t ticks = (us * SYSTICK_LOAD)-SYSTICK_DELAY_CALIB;
         while((start - SysTick->VAL) < ticks);
    } while (0);
}




