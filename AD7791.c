#include "AD7791.h"
#include <string.h>
#include <math.h>

#define SPI_TIMEOUT 10

static void spi_select(AD7791_t* self);
static void spi_deselect(AD7791_t* self);
static void spi_hw_command(AD7791_t *self, uint8_t cmd);

void AD7791_init(AD7791_t* self, SPI_HandleTypeDef* hspi,
		GPIO_TypeDef* CSport, uint16_t CSpin, double Vref)
{
	memset(self, 0, sizeof(*self));
	self->hspi = hspi;
	self->CSport  = CSport;
	self->CSpin = CSpin;
	self->Vref = Vref;
	self->bitResolution = 24;
	self->maxOutputValue = (uint32_t)pow(2, self->bitResolution);
}

void AD7791_setup(AD7791_t* self)
{
	/*
		1) See detailed registers descriptions in AD7791 documentation.

		2) Set any register option as function argument if necessary

		3) Following fixed options is implemented here:
			- Buffered mode
			- Unipolar mode
			- Non-divided internal clock
			- Burnout currents are disabled
			- Continuous convertion mode
			- fADC = 20 Hz
			- Rejection = 80dB
			- RMS noise = 1.5 uV
	*/

	uint8_t MR_word, BUF, UnB, BO, MD; // mode register options
	uint8_t FR_word, FS, CDIV; // filter register options

	// Filter register word
	FR_word = 0;
	FS = 0b011; // update rate
	CDIV = 0b00; // clock division factor

	FR_word = 0;
	FR_word += (FS << 0);
	FR_word += (CDIV << 4);

	// Mode register word
	BUF 	= 0b1; // 0 - unbuffered mode, 1 - buffered mode
	UnB 	= 0b1; // 0 - bipolar coding, 1 - unipolar coding
	BO 		= 0b0; // 0 - disable burnout current, 1 - enable burnout current
	MD 		= 0b00; // single - 0b10, continuous - 0b00

	MR_word = 0;
	MR_word += (BUF << 1);
	MR_word += (UnB << 2);
	MR_word += (BO << 3);
	MR_word += (MD << 6);


	spi_select(self);
	spi_hw_command(self, 0x20); // write to filter register
	spi_hw_command(self, FR_word);
	spi_deselect(self);
	HAL_Delay(10);

	spi_select(self);
	spi_hw_command(self, 0x10); // write to mode register
	spi_hw_command(self, MR_word);
	spi_deselect(self);
	HAL_Delay(10);
}


int32_t AD7791_get_dout_value(AD7791_t* self)
{
	const uint8_t kDataSizeBytes = 3;
	const uint8_t kBufferSizeBytes = 4;

	uint8_t rxBytes [kBufferSizeBytes];
	memset(rxBytes, 0, kBufferSizeBytes);

	spi_select(self);
	spi_hw_command(self, 0x38); // read from data register
	int i;
	for(i = 0; i < kDataSizeBytes; ++i)
	{
		HAL_SPI_Receive(self->hspi, rxBytes + kDataSizeBytes - i - 1, 1, SPI_TIMEOUT);
		while(HAL_SPI_GetState(self->hspi) != HAL_SPI_STATE_READY)
			;
	}
	spi_deselect(self);
	return *(int32_t*)rxBytes;
}

double AD7791_get_voltage(AD7791_t* self)
{
	return self->Vref * AD7791_get_dout_value(self) / self->maxOutputValue;
}


// Private methods
static void spi_select(AD7791_t* self)
{
	HAL_GPIO_WritePin(
			self->CSport,
			self->CSpin,
			GPIO_PIN_RESET
			);
}

static void spi_deselect(AD7791_t* self)
{
	HAL_GPIO_WritePin(
			self->CSport,
			self->CSpin,
			GPIO_PIN_SET
			);
}

static void spi_hw_command(AD7791_t *self, uint8_t cmd)
{
	HAL_SPI_Transmit(self->hspi, &cmd, 1, SPI_TIMEOUT);
	while(HAL_SPI_GetState(self->hspi) != HAL_SPI_STATE_READY)
		;
}


