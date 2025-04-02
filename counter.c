/*
 * counter.c
 *
 *  Created on: Dec 6, 2024
 *      Author: Kirill
 */


#include "counter.h"
#include <string.h>

void counter_init(counter_t* self, int64_t startValue)
{
	memset(self, 0, sizeof(*self));
	counter_stop(self);
	counter_set_value(self, startValue);
}

void counter_set_value(counter_t* self, int64_t value)
{
	self->value = value;
}

int64_t counter_get_value(counter_t* self)
{
	return self->value;
}

void counter_update(counter_t* self)
{
	self->value += self->step;
}

void counter_set_step(counter_t* self, int32_t step)
{
	self->step = step;
}

void counter_stop(counter_t* self)
{
	 counter_set_step(self, 0);
}

void counter_run_forward(counter_t* self)
{
	 counter_set_step(self, 1);
}

void counter_run_backward(counter_t* self)
{
	 counter_set_step(self, -1);
}

