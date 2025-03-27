#ifndef SPI_H_
#define SPI_H_
#include "stm32f10x.h"

#define TXBUF_SIZE				255
#define RXBUF_SIZE				TXBUF_SIZE

#define SPI1_CS_pin				GPIO_Pin_8
#define SPI1_RST_pin			GPIO_Pin_9
#define SPI1_SPI1_TXpin		(GPIO_Pin_3|GPIO_Pin_5)
#define SPI1_SPI1_RXpin		(GPIO_Pin_4)
#define SPI1_PORT					GPIOB

#define SPI1_CS_SELECT()				SPI1_PORT->BRR = SPI1_CS_pin
#define SPI1_CS_DESELECT()			SPI1_PORT->BSRR = SPI1_CS_pin
#define SPI1_RST_0()						SPI1_PORT->BRR = SPI1_RST_pin
#define SPI1_RST_1()						SPI1_PORT->BSRR = SPI1_RST_pin

void SPI_SendByte(uint8_t data);
uint8_t SPI_ReceiveByte(void);

#endif /* SPI_H_ */
