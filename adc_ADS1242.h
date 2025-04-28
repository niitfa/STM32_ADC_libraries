/*
 * adc_ADS1242.h
 *
 *  Created on: Apr 9, 2025
 *      Author: Kirill
 */

#ifndef SRC_ADC_ADC_ADS1242_H_
#define SRC_ADC_ADC_ADS1242_H_

/* SPI CONF
 * freq < 1.2 MHz (fosc = 4.9152 MHz)
 * freq < 600 kHz (fosc = 2.4576 MHz)
 * CPOL = LOW
 * CPHA = 2 (from 1 and 2)
 *
 * PDWN# can be tied to HIGH
 */

#include "adc.h"
#include "stm32f4xx_hal.h"

#define ADS1242_AIN_0 0b0000
#define ADS1242_AIN_1 0b0001
#define ADS1242_AIN_2 0b0010
#define ADS1242_AIN_3 0b0011

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
		);

#endif /* SRC_ADC_ADC_ADS1242_H_ */
