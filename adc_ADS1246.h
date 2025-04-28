/*
 * adc_ADS1246.h
 *
 *  Created on: Apr 2, 2025
 *      Author: Kirill
 */

#ifndef SRC_ADC_ADC_ADS1246_H_
#define SRC_ADC_ADC_ADS1246_H_

/* SPI CONF
 * freq = 1 MHz (internal clock)
 * CPOL = LOW
 * CPHA = 2 (from 1 and 2)
 *
 * START pin can be tied to HIGH
 * RESET# pin can be tied to HIGH
 */

#include "adc.h"
#include "stm32f4xx_hal.h"

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
		);


#endif /* SRC_ADC_ADC_ADS1246_H_ */
