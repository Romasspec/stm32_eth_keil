#include "net.h"
#include "arp.h"
#include "ip.h"
#include "string.h"
#include "uart.h"

uint8_t net_buf[ENC28J60_MAXFRAME];
extern uint8_t macaddr[6];
extern uint8_t ipaddr[4];
extern char str1[60];

void eth_send(enc28j60_frame_ptr *frame, uint16_t len)
{
  memcpy(frame->addr_dest,frame->addr_src,6);
  memcpy(frame->addr_src,macaddr,6);
  //enc28j60_packetSend((void*)frame,len + sizeof(enc28j60_frame_ptr));
	enc28j60_packetSend((void*)frame,len);
}

void eth_read(enc28j60_frame_ptr *frame, uint16_t len)
{
	if (len>=sizeof(enc28j60_frame_ptr))
	{
		if(frame->type==ETH_ARP){
			sprintf(str1,"%02X:%02X:%02X:%02X:%02X:%02X-%02X:%02X:%02X:%02X:%02X:%02X; %d; arp\r",
			frame->addr_src[0],frame->addr_src[1],frame->addr_src[2],frame->addr_src[3],frame->addr_src[4],frame->addr_src[5],
			frame->addr_dest[0],frame->addr_dest[1],frame->addr_dest[2],frame->addr_dest[3],frame->addr_dest[4],frame->addr_dest[5],
			len);
			uart1_send_buf((uint8_t*)str1, strlen(str1));
			if(arp_read(frame,len-sizeof(enc28j60_frame_ptr)))
			{
				arp_send(frame);
			}
			
		} else if(frame->type==ETH_IP){
			sprintf(str1,"%02X:%02X:%02X:%02X:%02X:%02X-%02X:%02X:%02X:%02X:%02X:%02X; %d; ip\r",
			frame->addr_src[0],frame->addr_src[1],frame->addr_src[2],frame->addr_src[3],frame->addr_src[4],frame->addr_src[5],
			frame->addr_dest[0],frame->addr_dest[1],frame->addr_dest[2],frame->addr_dest[3],frame->addr_dest[4],frame->addr_dest[5],
			len);
			uart1_send_buf((uint8_t*)str1, strlen(str1));
			ip_read(frame,len-sizeof(ip_pkt_ptr));
		}
	}
}

void net_pool(void)
{
	uint16_t len;
//	enc28j60_frame_ptr *frame=(void*)net_buf;
//	if ((len=enc28j60_packetReceive(net_buf,sizeof(net_buf)))>0)
//	{
//		eth_read(frame,len);
//	}
	
	enc28j60_frame_ptr	*ethFrame_ptr = (void*) net_buf;
	while ((len = enc28j60_packetReceive(net_buf,sizeof(net_buf))) > 0)
	{
		if (len >= sizeof(enc28j60_frame_ptr)) {
			if(ethFrame_ptr->type == ETH_ARP){
				if(arp_read(ethFrame_ptr,len-sizeof(enc28j60_frame_ptr)))
				{
					arp_send(ethFrame_ptr);					
				}
			} else if(ethFrame_ptr->type == ETH_IP){
				uint8_t res = ip_read(ethFrame_ptr,len-sizeof(ip_pkt_ptr));
				if (res == 1) {
					len = 0;
				}
			} else {
				len = 0;
			}
			
			
			if (len > 0) {
				eth_send((enc28j60_frame_ptr*) net_buf, len);
			}			
			
		}
		
	}
	
	
//	uart1_send(0x22);
//	uart1_send(0x33);
//	uart1_send(0x44);
//	uart1_send_buf(str1, 3);

}



