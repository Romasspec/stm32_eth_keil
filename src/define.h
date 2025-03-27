#ifndef DEFINE_H
#define DEFINE_H
#include "stm32f10x.h"
#include "time.h"
#include "ds18b20.h"
#include "tm1637.h"
#include "spi.h"
#include "enc28j60.h"
#include "net.h"
#include "uart.h"

//#include "MDR32F9Qx_config.h"
//#include "MDR32F9Qx_rst_clk.h"
//#include "MDR32F9Qx_port.h"
//#include "MDR32F9Qx_can.h"
//#include "ds18b20.h"
//#include "time.h"
//#include "tm1637.h"

#define PGN_DEV_RESET		((uint8_t) 0x3E)

#define LED_pin				GPIO_Pin_13
#define LED_PORT			GPIOC
#define LED_TOGLE()			(LED_PORT->ODR ^= LED_pin)
#define LED_ON()			(LED_PORT->ODR |= LED_pin)
#define LED_OFF()			(LED_PORT->ODR &=~LED_pin)



typedef struct
{
	union
	{
		struct
		{
			uint8_t sa :8;
			uint8_t	ps :8;
			uint8_t pf :8;
			uint8_t dp :1;
			uint8_t r  :1;
			uint8_t p  :3;
		};
		uint32_t	idt;
	};
	uint8_t len;
	union
	{
		uint8_t data[8] ;
		uint32_t data_u32[2];
	};

}j1939msg_t;

//void send_CAN(uint8_t *data, uint8_t lenght);
//void blink_reset (void);

void rcc_init(void);
void gpio_init (void);
void systic_init(void);
void spi1_init(void);
void uart1_init(void);

#endif
