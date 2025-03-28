#include "enc28j60.h"
#include <stdio.h>
#include <stdlib.h>
#include "string.h"

static uint8_t Enc28j60Bank = 0;
uint8_t macaddr[6] = MAC_ADDR;
static int gNextPacketPtr = 11;//RXSTART_INIT;
char buf[20]={0};

void enc28j60_writeOp(uint8_t op, uint8_t addres, uint8_t data)
{
	CS_SELECT();
	SPI_SendByte(op|(addres&ADDR_MASK));
	SPI_SendByte(data);
	CS_DESELECT();
}

uint8_t enc28j60_readOp(uint8_t op, uint8_t addres)
{
	uint8_t result;
	
	CS_SELECT();
	SPI_SendByte(op|(addres&ADDR_MASK));
	if(addres & 0x80) SPI_ReceiveByte();
	result=SPI_ReceiveByte();
	CS_DESELECT();
	return result;
}

static void enc28j60_readBuf(uint16_t len, uint8_t* data)
{
	CS_SELECT();
	SPI_SendByte(ENC28J60_READ_BUF_MEM);
	while(len--) {
		*data++ = SPI_ReceiveByte();
	}
	CS_DESELECT();
}

static void enc28j60_writeBuf(uint16_t len, uint8_t* data)
{
  CS_SELECT();
  SPI_SendByte(ENC28J60_WRITE_BUF_MEM);
  while(len--) {
		SPI_SendByte(*data++);
  }
  CS_DESELECT();
}

static void enc28j60_SetBank(uint8_t addres)
{
	if ((addres&BANK_MASK)!=Enc28j60Bank)
	{
		enc28j60_writeOp(ENC28J60_BIT_FIELD_CLR,ECON1,ECON1_BSEL1|ECON1_BSEL0);
		Enc28j60Bank = addres&BANK_MASK;
		enc28j60_writeOp(ENC28J60_BIT_FIELD_SET,ECON1,Enc28j60Bank>>5);
	}
}

static void enc28j60_writeRegByte(uint8_t addres, uint8_t data)
{
	enc28j60_SetBank(addres);
	enc28j60_writeOp(ENC28J60_WRITE_CTRL_REG, addres,data);
}

static uint8_t enc28j60_readRegByte(uint8_t addres)
{
	enc28j60_SetBank(addres);
	return enc28j60_readOp(ENC28J60_READ_CTRL_REG, addres);
}

static void enc28j60_writeReg(uint8_t addres, uint16_t data)
{
	enc28j60_writeRegByte(addres, data);
	enc28j60_writeRegByte(addres+1, data>>8);
}

static uint16_t enc28j60_readReg(uint8_t addres)
{
	return enc28j60_readRegByte(addres) | (enc28j60_readRegByte(addres + 1) << 8);
}
	
static void enc28j60_writePhy(uint8_t addres, uint16_t data)
{
	enc28j60_writeRegByte(MIREGADR, addres);
	enc28j60_writeReg(MIWR, data);
	while(enc28j60_readRegByte(MISTAT)&MISTAT_BUSY);
}

uint16_t enc28j60_packetReceive(uint8_t *buf, uint16_t buflen)
{
	uint16_t len = 0;
	if((enc28j60_readRegByte(EPKTCNT)>0))
	{		
		enc28j60_writeReg(ERDPT,gNextPacketPtr);
		uint16_t data = enc28j60_readReg(ERDPT);
		sprintf((char *)buf, "ERDPT = %02X\r", data);
		uart1_send_buf((uint8_t*)buf, strlen((char*)buf));
		
		struct{
		  uint16_t nextPacket;
		  uint16_t byteCount;
		  uint16_t status;
		} header;

		enc28j60_readBuf(sizeof header,(uint8_t*)&header);
		gNextPacketPtr=header.nextPacket;
		len = header.byteCount-4;//remove the CRC count
		sprintf((char *)buf, "ERDPT = %04X\r", gNextPacketPtr);
		uart1_send_buf((uint8_t*)buf, strlen((char*)buf));

		if(len > buflen) {
			len=buflen;
		}
		if((header.status&0x80)==0) {
			len=0;
		} else {
			enc28j60_readBuf(len, buf);
		}

		buf[len]=0;

		if(gNextPacketPtr-1 > RXSTOP_INIT) {
			enc28j60_writeReg(ERXRDPT,RXSTOP_INIT);
		} else if (gNextPacketPtr-1 < 0) {
			//enc28j60_writeReg(ERXRDPT,RXSTART_INIT);
			enc28j60_writeReg(ERDPT,RXSTART_INIT);
		} else {
			enc28j60_writeReg(ERXRDPT,gNextPacketPtr-1);
		}
		enc28j60_writeOp(ENC28J60_BIT_FIELD_SET,ECON2,ECON2_PKTDEC);
	}

	return len;
}

void enc28j60_packetSend(uint8_t *buf, uint16_t buflen)
{
	while(enc28j60_readOp(ENC28J60_READ_CTRL_REG,ECON1)&ECON1_TXRTS)		// Ждём готовности передатчика
	{
		if(enc28j60_readRegByte(EIR)& EIR_TXERIF){												// При ошибке, сбрасываем передатчик errata
			enc28j60_writeOp(ENC28J60_BIT_FIELD_SET,ECON1,ECON1_TXRST);
			enc28j60_writeOp(ENC28J60_BIT_FIELD_CLR,ECON1,ECON1_TXRST);
		}		
	}
	
//	enc28j60_writeReg(ETXST,TXSTART_INIT);
	enc28j60_writeReg(EWRPT,TXSTART_INIT);															// Устанавливаем указатели ETXST и ETXND
	enc28j60_writeReg(ETXND,TXSTART_INIT + buflen);
	
	enc28j60_writeBuf(1,(uint8_t*)"x00");																// Записываем пакет в буфер
	enc28j60_writeBuf(buflen,buf);
	
	enc28j60_writeOp(ENC28J60_BIT_FIELD_SET,ECON1,ECON1_TXRTS);
}

void enc28j60_init (void)
{	
	RST_0();
	delay_ms(2);
	RST_1();
	delay_ms(2);
	
	enc28j60_writeOp(ENC28J60_SOFT_RESET,0,ENC28J60_SOFT_RESET);
	delay_ms(2);
	//проверим, что всё перезагрузилось
	uint8_t dt = enc28j60_readOp(ENC28J60_READ_CTRL_REG,ESTAT);
	while(!(enc28j60_readOp(ENC28J60_READ_CTRL_REG,ESTAT)&ESTAT_CLKRDY));
	uart1_send(dt);
	
	enc28j60_writeReg(ERXST,RXSTART_INIT);
	enc28j60_writeReg(ERXRDPT,RXSTART_INIT);
	enc28j60_writeReg(ERXND,RXSTOP_INIT);
	enc28j60_writeReg(ETXST,TXSTART_INIT);
	enc28j60_writeReg(ETXND,TXSTOP_INIT);

	//Enable Broadcast
	enc28j60_writeRegByte(ERXFCON,enc28j60_readRegByte(ERXFCON)|ERXFCON_BCEN);

	//настраиваем канальный уровень
	enc28j60_writeRegByte(MACON1,MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS);
	enc28j60_writeRegByte(MACON2,0x00);
	enc28j60_writeOp(ENC28J60_BIT_FIELD_SET,MACON3,MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN);
	enc28j60_writeReg(MAIPG,0x0C12);
	enc28j60_writeRegByte(MABBIPG,0x12);//промежуток между фреймами
	enc28j60_writeReg(MAMXFL,MAX_FRAMELEN);//максимальный размер фрейма
	enc28j60_writeRegByte(MAADR5,macaddr[0]);//Set MAC addres
	enc28j60_writeRegByte(MAADR4,macaddr[1]);
	enc28j60_writeRegByte(MAADR3,macaddr[2]);
	enc28j60_writeRegByte(MAADR2,macaddr[3]);
	enc28j60_writeRegByte(MAADR1,macaddr[4]);
	enc28j60_writeRegByte(MAADR0,macaddr[5]);

	//настраиваем физический уровень
	enc28j60_writePhy(PHCON2,PHCON2_HDLDIS);//отключаем loopback
	enc28j60_writePhy(PHLCON,PHLCON_LACFG2| //светодиоды
	PHLCON_LBCFG2|PHLCON_LBCFG1|PHLCON_LBCFG0|
	PHLCON_LFRQ0|PHLCON_STRCH);

	enc28j60_SetBank(ECON1);
	enc28j60_writeOp(ENC28J60_BIT_FIELD_SET,EIE,EIE_INTIE|EIE_PKTIE);
	enc28j60_writeOp(ENC28J60_BIT_FIELD_SET,ECON1,ECON1_RXEN);//разрешаем приём пакетов
	
	uint16_t data = enc28j60_readReg(ERDPT);
	uart1_send_buf((uint8_t*)&data, 2);
}
