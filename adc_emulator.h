/*
 * adc_emulator.h
 *
 *  Created on: Dec 6, 2024
 *      Author: Kirill
 */

#ifndef SRC_ADC_ADC_EMULATOR_H_
#define SRC_ADC_ADC_EMULATOR_H_

#include "adc.h"

adc_t adc_emulator_create(double Vref, uint8_t bitResolution);


#endif /* SRC_ADC_ADC_EMULATOR_H_ */
