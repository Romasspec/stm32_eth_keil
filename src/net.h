#ifndef NET_H_
#define NET_H_
#include <stdio.h>
#include <stdlib.h>
#include "enc28j60.h"

#define IP_ADDR {192,168,0,197}

//--------------------------------------------------
typedef struct enc28j60_frame{
  uint8_t addr_dest[6];
  uint8_t addr_src[6];
  uint16_t type;
  uint8_t data[];
} enc28j60_frame_ptr;

typedef struct arp_msg{
  uint16_t net_tp;
  uint16_t proto_tp;
  uint8_t macaddr_len;
  uint8_t ipaddr_len;
  uint16_t op;
  uint8_t macaddr_src[6];
  uint8_t ipaddr_src[4];
  uint8_t macaddr_dst[6];
  uint8_t ipaddr_dst[4];
} arp_msg_ptr;

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


#define be16toword(a) ((((a)>>8)&0xff)|(((a)<<8)&0xff00))
#define ETH_ARP be16toword(0x0806)
#define ETH_IP be16toword(0x0800)
#define ARP_ETH be16toword(0x0001)
#define ARP_IP be16toword(0x0800)
#define ARP_REQUEST be16toword(1)
#define ARP_REPLY be16toword(2)

#define IP_ICMP 1
#define IP_TCP 6
#define IP_UDP 17

#define ICMP_REQ 8
#define ICMP_REPLY 0


void net_pool(void);
uint16_t enc28j60_packetReceive(uint8_t *buf,uint16_t buflen);

#endif /* NET_H_ */
