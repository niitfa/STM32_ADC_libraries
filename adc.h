/*
 * adc.h
 * Basic ADC class
 *  Created on: Dec 6, 2024
 *      Author: Kirill
 */

#ifndef SRC_ADC_ADC_H_
#define SRC_ADC_ADC_H_

#include <stdint.h>

struct adc_data_t;

typedef struct
{
	struct adc_vtable* vtable;
	struct adc_data_t* data;
} adc_t;

struct adc_vtable
{
	void (*init)(adc_t* self);
	void (*update)(adc_t* self, void* option);
	uint32_t (*get_cnt) (adc_t* self);
	double (*get_vout) (adc_t* self);
};

void adc_init(adc_t* self);
void adc_update(adc_t* self, void* option);
uint32_t adc_get_cnt(adc_t* self);
double adc_get_vout(adc_t* self);

#endif /* SRC_ADC_ADC_H_ */
