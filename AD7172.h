/*
 * adc_AD7172.h
 *
 *  Created on: Apr 28, 2025
 *      Author: Kirill
 */

#ifndef SRC_ADC_AD7172_H_
#define SRC_ADC_AD7172_H_

/* SPI CONF
 * freq <= 20 MHz
 * CPOL = HIGH
 * CPHA = 2 (from 1 and 2)
 */

#include "adc.h"
#include "stm32f4xx_hal.h"

#define AD7172_MAX_CHANNELS 4

typedef enum
{
	AD7172_WAIT,
	AD7172_MEASURE
} AD7172_state_t;

// heap allocation here!!!

typedef struct {
	SPI_HandleTypeDef* hspi;
	GPIO_TypeDef* portCS;
	uint16_t pinCS;
	uint8_t bitResolution;
	double Vref;
	int32_t lastOutputValue[AD7172_MAX_CHANNELS];
	int32_t maxOutputValue[AD7172_MAX_CHANNELS];
	AD7172_state_t state;
} AD7172_t;

int AD7172_init(AD7172_t *self);
void AD7172_update(AD7172_t *self);
int32_t AD7172_get_cnt(AD7172_t *self, int channel);
double AD7172_get_vout(AD7172_t *self, int channel);






#endif /* SRC_ADC_AD7172_H_ */
