/*
 * adc_ADS1246.h
 *
 *  Created on: Apr 2, 2025
 *      Author: Kirill
 */

#ifndef SRC_ADC_ADC_ADS1246_H_
#define SRC_ADC_ADC_ADS1246_H_

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
