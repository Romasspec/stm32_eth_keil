#ifndef ARP_H_
#define ARP_H_

#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "stm32f10x.h"
#include "net.h"

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


#define ARP_ETH be16toword(0x0001)
#define ARP_IP be16toword(0x0800)
#define ARP_REQUEST be16toword(1)
#define ARP_REPLY be16toword(2)

uint8_t arp_read(enc28j60_frame_ptr *frame, uint16_t len);
void arp_send(enc28j60_frame_ptr *frame);

#endif /*ARP_H_*/
