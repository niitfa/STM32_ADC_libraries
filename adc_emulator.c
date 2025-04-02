/*
 * adc_emulator.c
 *
 *  Created on: Dec 6, 2024
 *      Author: Kirill
 */

#include "adc_emulator.h"
#include "malloc.h"
#include <string.h>
#include <math.h>

struct adc_data_t
{
	uint8_t bitResolution;
	double Vref;
	uint32_t lastOutputValue;
	uint32_t maxOutputValue;
};

static void init(adc_t* self)
{
}

static void update(adc_t* self, void* option)
{
	if(option)
	{
		int32_t emulValue = *(int32_t*)option;

		if(emulValue > self->data->maxOutputValue)
			emulValue = self->data->maxOutputValue;

		if(emulValue < 0)
			emulValue = 0;

		self->data->lastOutputValue = emulValue;
	}
}

static uint32_t get_cnt(adc_t* self)
{
	return self->data->lastOutputValue;
}

static double get_vout(adc_t* self)
{
	return self->data->Vref * self->data->lastOutputValue / self->data->maxOutputValue;
}

static struct adc_vtable methods = {init, update, get_cnt, get_vout};

adc_t adc_emulator_create(double Vref, uint8_t bitResolution)
{
	adc_t adc;
	adc.vtable = &methods;
	struct adc_data_t* pdata =
			(struct adc_data_t*)malloc(sizeof(struct adc_data_t));
	if(pdata)
	{
		memset(pdata, 0, sizeof(*pdata));
		// fill data structure
		pdata->bitResolution = bitResolution;
		pdata->Vref = Vref;
		pdata->lastOutputValue = 0;
		pdata->maxOutputValue  = (uint32_t)pow(2, bitResolution);
		adc.data = pdata;
	}

	return adc;
}







