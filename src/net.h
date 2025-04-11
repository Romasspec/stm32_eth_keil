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
  uint8_t data[1];
} enc28j60_frame_ptr;


#define be16toword(a) ((((a)>>8)&0xff)|(((a)<<8)&0xff00))
#define be32todword(a) ((((a)>>24)&0xff)|(((a)>>8)&0xff00)|(((a)<<8)&0xff0000)|(((a)<<24)&0xff000000))
#define ETH_ARP be16toword(0x0806)
#define ETH_IP be16toword(0x0800)


void net_pool(void);
uint16_t enc28j60_packetReceive(uint8_t *buf,uint16_t buflen);
void eth_send(enc28j60_frame_ptr *frame, uint16_t len);

#endif /* NET_H_ */
