#include "tcp.h"

extern char str1[60];
extern uint8_t net_buf[ENC28J60_MAXFRAME];

uint8_t tcp_send(uint8_t *ip_addr, uint16_t port, uint8_t op)
{
  uint8_t res=0;
  uint16_t len=0;
	
	static uint32_t num_seg=0;
	enc28j60_frame_ptr *frame=(void*) net_buf;
	ip_pkt_ptr *ip_pkt = (void*)(frame->data);
	tcp_pkt_ptr *tcp_pkt = (void*)(ip_pkt->data);
	
	if (op==TCP_OP_SYNACK)
	{
		//Заполним заголовок пакета TCP
  tcp_pkt->port_dst = be16toword(port);
  tcp_pkt->port_src = be16toword(LOCAL_PORT_TCP);
  tcp_pkt->num_ask = be32todword(be32todword(tcp_pkt->bt_num_seg) + 1);
  tcp_pkt->bt_num_seg = rand();
  tcp_pkt->fl = TCP_SYN | TCP_ACK;
  tcp_pkt->size_wnd = be16toword(8192);
  tcp_pkt->urg_ptr = 0;
  len = sizeof(tcp_pkt_ptr)+4;
  tcp_pkt->len_hdr = len << 2;
  tcp_pkt->data[0]=2;//Maximum Segment Size (2)
  tcp_pkt->data[1]=4;//Length
  tcp_pkt->data[2]=0x05;
  tcp_pkt->data[3]=0x82;
  tcp_pkt->cs = be16toword(IP_TCP + len);
  tcp_pkt->cs=checksum((uint8_t*)tcp_pkt-8, len+8);
	}

  return res;
}

uint8_t tcp_read (enc28j60_frame_ptr *frame, uint16_t len)
{
	uint8_t res=0;
	
	sprintf(str1,"%02X:%02X:%02X:%02X:%02X:%02X-%02X:%02X:%02X:%02X:%02X:%02X; %d tcp\r\n",
  frame->addr_src[0],frame->addr_src[1],frame->addr_src[2],
  frame->addr_src[3],frame->addr_src[4],frame->addr_src[5],
  frame->addr_dest[0],frame->addr_dest[1],frame->addr_dest[2],
  frame->addr_dest[3],frame->addr_dest[4],frame->addr_dest[5],len);
	uart1_send_buf((uint8_t*)str1, strlen(str1));
	
	ip_pkt_ptr *ip_pkt = (void*)(frame->data);
	tcp_pkt_ptr *tcp_pkt = (void*)(ip_pkt->data);

	if (tcp_pkt->fl == TCP_SYN)
	{
		tcp_send(ip_pkt->ipaddr_src, be16toword(tcp_pkt->port_src), TCP_OP_SYNACK);
	}

  return res;
}

