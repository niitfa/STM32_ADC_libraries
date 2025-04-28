/*
 * adc_AD7172.c
 *
 *  Created on: Apr 28, 2025
 *      Author: Kirill
 */


#include <AD7172.h>
#include <malloc.h>
#include <string.h>
#include <math.h>

#define ADC_AD7172_SPI_TIMEOUT 10

static void spi_select(AD7172_t* self);
static void spi_deselect(AD7172_t* self);
static void spi_hw_command(AD7172_t *self, uint8_t cmd);
static void check_negative_24_to_32(int32_t* val);

int AD7172_init(AD7172_t *self)
{
	return 0;
}

void AD7172_update(AD7172_t *self)
{
	// write to offset0_reg
	// write to gain0_reg
	// write to filter0_reg
	// write to setup0_reg

	// write to ch0_reg (setup / pos = ch0, neg = ch4)
	// write to ch1_reg (setup / pos = ch1, neg = ch4)
	// write to ch2_reg (setup / pos = ch2, neg = ch4)
	// write to ch3_reg (setup / pos = ch3, neg = ch4)


	// reading data
}

void AD7172_get_cnt(AD7172_t *self, int channel)
{
	if(channel >= 0 && channel < AD7172_MAX_CHANNELS)
	{
		return self->lastOutputValue[channel];
	}
}

void AD7172_get_vout(AD7172_t *self, int channel)
{
	return self->Vref * ((double)AD7172_get_cnt(self) / self->maxOutputValue);
}


static void spi_select(AD7172_t* self)
{
	HAL_GPIO_WritePin(
			self->portCS,
			self->pinCS,
			GPIO_PIN_RESET
			);
}
static void spi_deselect(AD7172_t* self)
{
	HAL_GPIO_WritePin(
			self->portCS,
			self->pinCS,
			GPIO_PIN_SET
			);
}
static void spi_hw_command(AD7172_t *self, uint8_t cmd)
{
	HAL_SPI_Transmit(self->hspi, &cmd, 1, ADC_AD7172_SPI_TIMEOUT);
	while(HAL_SPI_GetState(self->hspi) != HAL_SPI_STATE_READY)
		;
}
static void check_negative_24_to_32(int32_t* val)
{
	if ((*val >> 23) & (int)1)
	{
		*val |= 0xFF000000;
	}
}
