/* Example:
* 
 * my_program.c
 * 
 * #include "ADC7791.h"
 *
 * extern SPI_HandleTypeDef hspi1;
 *
 * void my_program()
 * {
 * 		AD7791_t adc;
 * 
 * 		AD7791_init(&adc, &hspi1, GPIOD, GPIO_PIN_0, 2.5);
 * 		AD7791_setup(&adc);
 *
 * 		while(1)
 * 		{
 * 			int32_t adc_digital_value = AD7791_get_dout_value(&adc);
 * 			double adc_analog_value = AD7791_get_voltage(&adc);
 * 			HAL_Delay(50);
 * 		}
 * }
 *
 */

#ifndef AD7791_H
#define AD7791_H

#include "stm32f4xx_hal.h"

typedef struct
{
	SPI_HandleTypeDef* hspi;
	GPIO_TypeDef* CSport;
	uint16_t CSpin;
	uint8_t bitResolution;
	double Vref;
	int32_t lastOutputValue;
	int32_t maxOutputValue;

} AD7791_t;

void AD7791_init(AD7791_t* self,
		SPI_HandleTypeDef* hspi,
		GPIO_TypeDef* CSport,
		uint16_t CSpin,
		double Vref);
void AD7791_setup(AD7791_t* self);
int32_t AD7791_get_dout_value(AD7791_t* self);
double AD7791_get_voltage(AD7791_t* self);

#endif
