#ifndef SRC_UART_H_
#define SRC_UART_H_
#include "stm32f10x.h"



void uart1_send (uint8_t data);
void uart_pool(void);
void uart1_send_buf(uint8_t* str, uint8_t strlen);
#endif /* SRC_UART_H_ */
