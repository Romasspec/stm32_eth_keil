#include "spi.h"

uint8_t spi1_rx_buf[RXBUF_SIZE];
uint8_t spi1_tx_buf[TXBUF_SIZE];
uint8_t rx_buf_head = 0;
uint8_t rx_buf_tail = 0;
uint8_t tx_index; 			//тут хранится количество переданных байт
uint8_t tx_len;   			//сколько всего байт нужно передать
uint8_t *tx_data;     	//указатель на массив с передаваемыми данными

static uint8_t SPI1_WriteRead(uint8_t Byte)
{
	uint8_t receivedbyte = 0;
	while (!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE));
	SPI_I2S_SendData(SPI1, Byte);
	while (!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));
	receivedbyte = (uint8_t)(SPI_I2S_ReceiveData(SPI1));
	return receivedbyte;
}

void SPI_SendByte(uint8_t data)
{
//	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY)) {};
//	tx_index = 0;
//	tx_len = len;
//	tx_data = data;
//	SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_TXE, ENABLE);
	SPI1_WriteRead(data);
}

uint8_t SPI_ReceiveByte(void)
{
//	uint8_t bt = spi1_rx_buf[rx_buf_tail];
//	rx_buf_tail++;
//	rx_buf_tail = rx_buf_tail+1 % TXBUF_SIZE;
	uint8_t bt = SPI1_WriteRead(0x00);
	return bt; //вернём регистр данных
}

void SPI1_IRQHandler (void)
{
	if(SPI_I2S_GetITStatus(SPI1, SPI_I2S_IT_TXE)) {
		SPI1_CS_SELECT();
		SPI_I2S_SendData(SPI1, tx_data[tx_index]);
		tx_index++;
		if (tx_index >= tx_len) {
			SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_TXE, DISABLE);
			while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY)) {};
			SPI1_CS_DESELECT();
			NVIC_ClearPendingIRQ(SPI1_IRQn);
		}
		SPI_I2S_ClearITPendingBit(SPI1, SPI_I2S_IT_TXE);
	}

	if(SPI_I2S_GetITStatus(SPI1, SPI_I2S_IT_RXNE)) {
//		GPIOC->ODR ^= GPIO_Pin_14;
		spi1_rx_buf[rx_buf_head++] = (uint8_t)(SPI_I2S_ReceiveData(SPI1));
		SPI_I2S_ClearITPendingBit(SPI1, SPI_I2S_IT_RXNE);
	}
}
