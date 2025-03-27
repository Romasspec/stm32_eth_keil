#ifndef TM1637_H_
#define TM1637_H_

//#include "MDR32F9Qx_config.h"
//#include "MDR32F9Qx_port.h"
#include "stm32f10x.h"
#include "time.h"

#define TM1637_DIO_PIN 			GPIO_Pin_14
#define TM1637_CLK_PIN 			GPIO_Pin_13
#define TM1637_PORT					GPIOA
//#define TM1637_CLK_1()		TM1637_PORT->RXTX |= TM1637_CLK_PIN
//#define TM1637_CLK_0()		TM1637_PORT->RXTX &=~TM1637_CLK_PIN
//#define TM1637_DIO_1()		TM1637_PORT->RXTX |= TM1637_DIO_PIN
//#define TM1637_DIO_0()		TM1637_PORT->RXTX &=~TM1637_DIO_PIN

#define WRITE_DATA_REGISTER_AUTOM_ADDR	0x40
#define READ_DATA_REGISTER_AUTOM_ADDR	0x42
#define WRITE_DATA_REGISTER_FIX_ADDR	0x44
#define READ_DATA_REGISTER_FIX_ADDR		0x46
#define ACK_TIMEOUT						500

enum {
	DIG1 = 0,
	DIG2,
	DIG3,
	DIG4
};

enum {
	BRIGHTNESS0 = 0,
	BRIGHTNESS1,
	BRIGHTNESS2,
	BRIGHTNESS3,
	BRIGHTNESS4,
	BRIGHTNESS5,
	BRIGHTNESS6,
	BRIGHTNESS7,
	BRIGHTNESS8
};

void tm1637_start (void);
void tm1637_stop (void);
void tm1637_write_byte (uint8_t byte);
void tm1637_send_byte (uint8_t dig, uint8_t byte);
uint8_t digToHEX(uint8_t digit);
void tm1637_set_brightness (uint8_t brightness);
void tm1637_send_buf (uint8_t *buf, uint8_t lenght);

#endif /* TM1637_H_ */
