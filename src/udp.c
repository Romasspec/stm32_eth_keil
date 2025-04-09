#include "udp.h"
#include "ip.h"

extern char str1[60];

uint8_t udp_read(enc28j60_frame_ptr *frame, uint16_t len)
{
	uint8_t res=0;
	
	ip_pkt_ptr *ip_pkt = (void*)frame->data;
	udp_pkt_ptr *udp_pkt = (void*)ip_pkt->data;
	
	
	sprintf(str1,"UDP_srcPort %d; UDP_desPort %d; UDP_len %d; UDP_CS %d\r",
				be16toword(udp_pkt->srcPort),be16toword(udp_pkt->destPort),be16toword(udp_pkt->len),be16toword(udp_pkt->checkSum));
				uart1_send_buf((uint8_t*)str1, strlen(str1));
	
	
	
	
	if (/*(len>=sizeof(udp_pkt))&&*/be16toword(udp_pkt->destPort) == UDP_DEMO_PORT) {
		sprintf(str1,"udp request\r");
		uart1_send_buf((uint8_t*)str1, strlen(str1));
		
		sprintf(str1,"%u-%u\r\n", be16toword(udp_pkt->srcPort),be16toword(udp_pkt->destPort));
		
		uart1_send_buf((uint8_t*)str1, strlen(str1));
		uart1_send_buf(udp_pkt->data, len - sizeof(udp_pkt_ptr));
		uart1_send_buf((uint8_t*)"\r\n", 2);		
		
		for(uint16_t i = 0 ; i < len - sizeof(udp_pkt_ptr) - 1; i++){
      udp_pkt->data[i]++;
    }
		
		uint16_t swapPort = udp_pkt->destPort;
		udp_pkt->destPort = udp_pkt->srcPort;
		udp_pkt->srcPort = swapPort;
		udp_pkt->checkSum = 0;
//		udp_pkt->checkSum = checksum((void*)udp_pkt, len);
	}
	
	return res;
}
