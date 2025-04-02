/*
 * acd.c
 *
 *  Created on: Dec 6, 2024
 *      Author: Kirill
 */


#include "adc.h"

void adc_init(adc_t* self)
{
	self->vtable->init(self);
}

void adc_update(adc_t* self, void* option)
{
	self->vtable->update(self, option);
}

uint32_t adc_get_cnt(adc_t* self)
{
	return self->vtable->get_cnt(self);
}

double adc_get_vout(adc_t* self)
{
	return self->vtable->get_vout(self);
}
