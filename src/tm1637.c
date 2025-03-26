#include "tm1637.h"

const uint8_t digitHEX[] = {0x3f, 0x06, 0x5b, 0x4f,
							0x66, 0x6d, 0x7d, 0x07,
							0x7f, 0x6f, 0x00, 0x40
};							//0~9, ,-

static inline void TM1637_CLK_1(void)
{
	TM1637_PORT->ODR |= TM1637_CLK_PIN;
}

static inline void TM1637_CLK_0(void)
{
	TM1637_PORT->ODR &=~TM1637_CLK_PIN;
}

static inline void TM1637_DIO_1(void)
{
	TM1637_PORT->ODR |= TM1637_DIO_PIN;
}

static inline void TM1637_DIO_0(void)
{
	TM1637_PORT->ODR &=~TM1637_DIO_PIN;
}

static inline uint8_t TM1637_DIO_IN (void)
{
	return (TM1637_PORT->ODR & TM1637_DIO_PIN);
}

uint8_t digToHEX(uint8_t digit) {
    return digitHEX[digit];
}

void tm1637_start (void)
{
	TM1637_CLK_1();
	TM1637_DIO_1();
	delayMicroseconds(0);
	TM1637_DIO_0();
//	TM1637_CLK_0();
}

void tm1637_stop (void)
{
	TM1637_CLK_0();
	TM1637_DIO_0();
	delayMicroseconds(0);
	TM1637_CLK_1();
	delayMicroseconds(0);
	TM1637_DIO_1();
}

void tm1637_write_byte (uint8_t byte)
{
	uint8_t i;
	uint32_t timeout = ACK_TIMEOUT;

	for(i=0; i<8; i++) {
		delayMicroseconds(1);
		TM1637_CLK_0();
		delayMicroseconds(1);
		if (byte & 0x01) {
			TM1637_DIO_1();
		} else {
			TM1637_DIO_0();
		}
		delayMicroseconds(1);
		TM1637_CLK_1();
		byte >>= 1;
	}

	delayMicroseconds(1);
	TM1637_CLK_0();
	delayMicroseconds(1);
	TM1637_DIO_1();
	while (TM1637_DIO_IN() && timeout--);
	TM1637_CLK_1();
}

void tm1637_send_byte (uint8_t dig, uint8_t byte)
{
	tm1637_start();
	tm1637_write_byte(WRITE_DATA_REGISTER_FIX_ADDR);
	tm1637_stop();

	tm1637_start();
	tm1637_write_byte(dig | 0xC0);
	tm1637_write_byte(byte);
	tm1637_stop();
}

void tm1637_send_buf (uint8_t *buf, uint8_t lenght)
{
	uint8_t i;
	tm1637_start();
	tm1637_write_byte(WRITE_DATA_REGISTER_AUTOM_ADDR);
	tm1637_stop();

	tm1637_start();
	tm1637_write_byte(0xC0);
	for(i=0; i<lenght; i++) {
		tm1637_write_byte(*(buf+i));
	}
	tm1637_stop();
}


void tm1637_set_brightness (uint8_t brightness)
{
	tm1637_start();
	if (brightness == BRIGHTNESS0) {
		tm1637_write_byte(0x80);
	} else {
		tm1637_write_byte(brightness | 0x88);
	}
	tm1637_stop();
}
