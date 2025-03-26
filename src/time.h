#ifndef TIME_H_
#define TIME_H_
//#include "MDR32F9Qx_config.h"
#include "stm32f10x.h"

void systic_init(void);
uint32_t micros(void);
uint32_t millis(void);
void delayMicroseconds(uint32_t us);
void delay_ms(uint32_t ms);

#endif /* TIME_H_ */
