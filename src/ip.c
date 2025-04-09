#include "ip.h"
#include "udp.h"

extern uint8_t ipaddr[4];
extern char str1[60];

uint16_t checksum(uint8_t *ptr, uint16_t len)
{
	uint32_t sum=0;
	while(len>1){
		sum += (uint16_t) (((uint32_t)*ptr<<8)|*(ptr+1));
		ptr+=2;
		len-=2;
	}
	if(len){
		sum+=((uint32_t)*ptr)<<8;
	}
	while (sum>>16) sum=(uint16_t)sum+(sum>>16);
	return ~be16toword((uint16_t)sum);
}

uint8_t ip_send(enc28j60_frame_ptr *frame, uint16_t len)
{
	uint8_t res=0;
	ip_pkt_ptr *ip_pkt = (void*)frame->data;
	
	//Заполним заголовок пакета IP
	ip_pkt->len=be16toword(len);
	ip_pkt->fl_frg_of=0;
	ip_pkt->ttl=128;
	ip_pkt->cs = 0;
	memcpy(ip_pkt->ipaddr_dst,ip_pkt->ipaddr_src,4);
	memcpy(ip_pkt->ipaddr_src,ipaddr,4);
	ip_pkt->cs = checksum((void*)ip_pkt,sizeof(ip_pkt_ptr));
	
	//eth_send(frame,len);
	return res;
}

uint8_t icmp_read(enc28j60_frame_ptr *frame, uint16_t len)
{
	uint8_t res=0;
	ip_pkt_ptr *ip_pkt = (void*)frame->data;
	icmp_pkt_ptr *icmp_pkt = (void*)ip_pkt->data;
	
	if ((len>=sizeof(icmp_pkt_ptr))&&(icmp_pkt->msg_tp==ICMP_REQ)){
		sprintf(str1,"icmp request\r");
		uart1_send_buf((uint8_t*)str1, strlen(str1));
		
		icmp_pkt->msg_tp=ICMP_REPLY;
		icmp_pkt->cs=0;
		icmp_pkt->cs=checksum((void*)icmp_pkt,len);
//		ip_send(frame,len+sizeof(ip_pkt_ptr));
	}	

	return res;
}

uint8_t ip_read(enc28j60_frame_ptr *frame, uint16_t len)
{
	uint8_t res=0;
	ip_pkt_ptr *ip_pkt = (void*)(frame->data);
	
	if((ip_pkt->verlen==0x45)&&(!memcmp(ip_pkt->ipaddr_dst,ipaddr,4)))
	{
		uint16_t rxCheckSum = ip_pkt->cs;
		ip_pkt->cs = 0;
		uint16_t calcCheckSum = checksum ((uint8_t*)ip_pkt, sizeof(ip_pkt_ptr));
		
		if (rxCheckSum == calcCheckSum) {
			len = be16toword(ip_pkt->len) - sizeof(ip_pkt_ptr);				//длина данных
//		sprintf(str1,"rnip_cs 0x%04X\r\n", ip_pkt->cs);
//		uart1_send_buf((uint8_t*)str1, strlen(str1));
//		
//		ip_pkt->cs=0;
//		sprintf(str1,"ip_cs 0x%04Xrn", checksum((void*)ip_pkt,sizeof(ip_pkt_ptr)));
//		uart1_send_buf((uint8_t*)str1, strlen(str1));
			if (ip_pkt->prt==IP_ICMP)
			{
				sprintf(str1,"ICMP paket\r\n");
				uart1_send_buf((uint8_t*)str1, strlen(str1));
				icmp_read(frame,len);
				ip_send(frame,len+sizeof(ip_pkt_ptr));
			} else if (ip_pkt->prt==IP_TCP){
				sprintf(str1,"TCP IP paket\r\n");
				uart1_send_buf((uint8_t*)str1, strlen(str1));
			} else if (ip_pkt->prt==IP_UDP){
				sprintf(str1,"UDP paket\r\n");
				uart1_send_buf((uint8_t*)str1, strlen(str1));
				udp_read(frame,len);
				ip_send(frame,len+sizeof(ip_pkt_ptr));
			}
			
		}
		
}
  return res;
  
}
