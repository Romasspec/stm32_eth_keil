#ifndef UDP_H
#define UDP_H
#include <stdio.h>
#include "stm32f10x.h"
#include "string.h"
#include "net.h"

typedef struct UDP_Frame
{
	uint16_t srcPort;
	uint16_t destPort;
	uint16_t len;
	uint16_t checkSum;
	uint8_t data[];
} udp_pkt_ptr;

#define UDP_DEMO_PORT						33333

uint8_t udp_read(enc28j60_frame_ptr *frame, uint16_t len);

#endif /*UDP_H*/
