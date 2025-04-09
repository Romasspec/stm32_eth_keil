#ifndef IP_H
#define IP_H
#include <stdio.h>
#include "stm32f10x.h"
#include "string.h"
#include "net.h"


typedef struct ip_pkt{
uint8_t verlen;					//версия протокола и длина заголовка
uint8_t ts;							//тип севриса
uint16_t len;						//длина
uint16_t id;						//идентификатор пакета
uint16_t fl_frg_of;			//флаги и смещение фрагмента
uint8_t ttl;						//время жизни
uint8_t prt;						//тип протокола
uint16_t cs;						//контрольная сумма заголовка
uint8_t ipaddr_src[4];	//IP-адрес отправителя
uint8_t ipaddr_dst[4];	//IP-адрес получателя
uint8_t data[];					//данные
} ip_pkt_ptr;

typedef struct icmp_pkt{
uint8_t msg_tp;					//тип севриса
uint8_t msg_cd;					//код сообщения
uint16_t cs;						//контрольная сумма заголовка
uint16_t id;						//идентификатор пакета
uint16_t num;						//номер пакета
uint8_t data[];					//данные
} icmp_pkt_ptr;

#define IP_ICMP 1
#define IP_TCP 6
#define IP_UDP 17

#define ICMP_REQ 8
#define ICMP_REPLY 0

uint8_t ip_read(enc28j60_frame_ptr *frame, uint16_t len);
uint8_t icmp_read(enc28j60_frame_ptr *frame, uint16_t len);
uint16_t checksum(uint8_t *ptr, uint16_t len);
uint8_t ip_send(enc28j60_frame_ptr *frame, uint16_t len);

#endif /*IP_H*/
