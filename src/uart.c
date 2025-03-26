#include "uart.h"
uint8_t rx_data;
uint8_t flag = 0;
#include "time.h"

void uart1_send (uint8_t data)
{
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
	{
		GPIOC->ODR ^= GPIO_Pin_14;
	}	
	USART_SendData(USART1, data);
}

void uart1_send_buf(uint8_t* str, uint8_t strlen)
{
	while (strlen--) {
		uart1_send (*str);
		str++;		
	}
}

void uart_pool(void)
{
	if (flag == 1)
	{
		flag = 0;
		if(rx_data == 'R') {
			NVIC_SystemReset();
		}
	}
}

void USART1_IRQHandler (void)
{

	if (USART_GetITStatus(USART1, USART_IT_RXNE))
	{
		flag = 1;
		rx_data = (uint8_t) USART_ReceiveData(USART1);
	}
	if (USART_GetITStatus(USART1, USART_IT_TC))
	{
		USART_ClearITPendingBit(USART1, USART_IT_TC);
	}
	if (USART_GetITStatus(USART1, USART_IT_TXE))
	{
		USART_ClearITPendingBit(USART1, USART_IT_TXE);
	}
}
