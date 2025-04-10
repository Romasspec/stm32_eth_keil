#ifndef TCP_H
#define TCP_H
#include <stdio.h>
#include "stm32f10x.h"
#include "string.h"
#include "net.h"
#include "ip.h"

typedef struct tcp_pkt {
  uint16_t port_src;			//порт отправителя
  uint16_t port_dst;			//порт получателя
  uint32_t bt_num_seg;		//порядковый номер байта в потоке данных (указатель на первый байт в сегменте данных)
  uint32_t num_ask;				//номер подтверждения (первый байт в сегменте + количество байтов в сегменте + 1 или номер следующего ожидаемого байта)
  uint8_t len_hdr;				//длина заголовка
  uint8_t fl;							//флаги TCP
  uint16_t size_wnd;			//размер окна
  uint16_t cs;						//контрольная сумма заголовка
  uint16_t urg_ptr;				//указатель на срочные данные
  uint8_t data[];					//данные
} tcp_pkt_ptr;

#define LOCAL_PORT_TCP 80
//--------------------------------------------------
//флаги TCP
#define TCP_CWR 0x80
#define TCP_ECE 0x40
#define TCP_URG 0x20
#define TCP_ACK 0x10
#define TCP_PSH 0x08
#define TCP_RST 0x04
#define TCP_SYN 0x02
#define TCP_FIN 0x01
//--------------------------------------------------
//операции TCP
#define TCP_OP_SYNACK 1
#define TCP_OP_ACK_OF_FIN 2
#define TCP_OP_ACK_OF_RST 3
//--------------------------------------------------

uint8_t tcp_read (enc28j60_frame_ptr *frame, uint16_t len);
uint8_t tcp_send(uint8_t *ip_addr, uint16_t port, uint8_t op);

#endif /*TCP_H*/
