/*
 * counter.h
 *
 *  Created on: Dec 6, 2024
 *      Author: Kirill
 */

#ifndef SRC_ADC_COUNTER_H_
#define SRC_ADC_COUNTER_H_

#include <stdint.h>

typedef struct
{
	int32_t step;
	int64_t value;
} counter_t;

void counter_init(counter_t* self, int64_t startValue);
void counter_set_value(counter_t* self, int64_t value);
int64_t counter_get_value(counter_t* self);
void counter_update(counter_t* self);
void counter_set_step(counter_t* self, int32_t step);
void counter_stop(counter_t* self);
void counter_run_forward(counter_t* self);
void counter_run_backward(counter_t* self);




#endif /* SRC_ADC_COUNTER_H_ */
