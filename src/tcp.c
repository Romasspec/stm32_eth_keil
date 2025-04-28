#include "tcp.h"

extern char str1[60];
extern uint8_t net_buf[ENC28J60_MAXFRAME];
extern uint8_t ipaddr[4];
extern uint8_t macaddr[6];

tcp_prop_ptr tcpprop;
volatile uint16_t tcp_mss = 458;
volatile uint8_t tcp_stat = TCP_DISCONNECTED;

const char http_header[] = {"HTTP/1.1 200 OKrnServer: nginxrnContent-Type: text/htmlrnConnection: keep-alivernrn"};
const uint8_t index_htm[] = {
0x3c,0x68,0x74,0x6d,0x6c,0x3e,0x3c,0x62,0x6f,0x64,0x79,0x3e,0x3c,0x68,0x31,0x20,
0x73,0x74,0x79,0x6c,0x65,0x3d,0x22,0x74,0x65,0x78,0x74,0x2d,0x61,0x6c,0x69,0x67,
0x6e,0x3a,0x20,0x63,0x65,0x6e,0x74,0x65,0x72,0x3b,0x22,0x3e,0x53,0x54,0x4d,0x33,
0x32,0x46,0x31,0x30,0x33,0x78,0x38,0x3c,0x62,0x72,0x3e,0x3c,0x62,0x72,0x3e,0x57,
0x45,0x42,0x20,0x53,0x65,0x72,0x76,0x65,0x72,0x3c,0x2f,0x68,0x31,0x3e,0x0a,0x3c,
0x70,0x3e,0x3c,0x2f,0x70,0x3e,0x0a,0x3c,0x68,0x32,0x3e,0x46,0x65,0x61,0x74,0x75,
0x72,0x65,0x73,0x3c,0x2f,0x68,0x32,0x3e,0x0a,0x3c,0x70,0x3e,0x41,0x52,0x4d,0xc2,
0xae,0x20,0x33,0x32,0x2d,0x62,0x69,0x74,0x20,0x43,0x6f,0x72,0x74,0x65,0x78,0xc2,
0xae,0x2d,0x4d,0x33,0x20,0x43,0x50,0x55,0x20,0x43,0x6f,0x72,0x65,0x3c,0x2f,0x70,
0x3e,0x0a,0x3c,0x2f,0x62,0x6f,0x64,0x79,0x3e,0x3c,0x2f,0x68,0x74,0x6d,0x6c,0x3e
};

//Подготовка заголовка TCP-пакета
void tcp_header_prepare(tcp_pkt_ptr *tcp_pkt, uint16_t port, uint8_t fl, uint16_t len)
{
	tcp_pkt->port_dst = be16toword(port);
  tcp_pkt->port_src = be16toword(LOCAL_PORT_TCP);
  tcp_pkt->bt_num_seg = tcpprop.seq_num;
  tcp_pkt->num_ask = tcpprop.ack_num;
  tcp_pkt->fl = fl;
  tcp_pkt->size_wnd = be16toword(8192);
  tcp_pkt->urg_ptr = 0;
  tcp_pkt->len_hdr = len << 2;
  tcp_pkt->cs = be16toword(IP_TCP + len);
  tcp_pkt->cs=checksum((uint8_t*)tcp_pkt-8, len+8);
}


//Подготовка заголовка IP-пакета

void ip_header_prepare(ip_pkt_ptr *ip_pkt, uint8_t *ip_addr, uint8_t prt, uint16_t len)
{
	ip_pkt->len=be16toword(len);
	ip_pkt->id = 0;
	ip_pkt->ts = 0;
	ip_pkt->verlen = 0x45;
	ip_pkt->fl_frg_of=0;
	ip_pkt->ttl=128;
	ip_pkt->cs = 0;
	ip_pkt->prt=prt;
	memcpy(ip_pkt->ipaddr_dst,ip_addr,4);
	memcpy(ip_pkt->ipaddr_src,ipaddr,4);
	ip_pkt->cs = checksum((void*)ip_pkt,sizeof(ip_pkt_ptr));	
}

/*Отправка ответа на запрос соединения*/
uint8_t tcp_send_synack(enc28j60_frame_ptr *frame, uint8_t *ip_addr, uint16_t port)
{
	uint8_t res=1;
	uint16_t len=0;
	ip_pkt_ptr *ip_pkt = (void*)(frame->data);
	tcp_pkt_ptr *tcp_pkt = (void*)(ip_pkt->data);
	
	tcpprop.seq_num = rand();
	tcpprop.ack_num = be32todword(be32todword(tcp_pkt->bt_num_seg) + 1);
	tcp_pkt->data[0]=2;//Maximum Segment Size (2)
	tcp_pkt->data[1]=4;//Length
	tcp_pkt->data[2]=(uint8_t) (tcp_mss>>8);//MSS = 458
	tcp_pkt->data[3]=(uint8_t) tcp_mss;
	len = sizeof(tcp_pkt_ptr)+4;
	tcp_header_prepare(tcp_pkt, port, TCP_SYN|TCP_ACK, len);

	len+=sizeof(ip_pkt_ptr);
	ip_header_prepare(ip_pkt, ip_addr, IP_TCP, len);

	//Заполним заголовок Ethernet
	frame->type = ETH_IP;
	len+=sizeof(enc28j60_frame_ptr);
	eth_send(frame,len);
	
	tcp_stat = TCP_CONNECTED;
	return res;
}

/*Отправка ответа на запрос разъединения и затем отправка такого же запроса при условии*/

uint8_t tcp_send_finack(enc28j60_frame_ptr *frame, uint8_t *ip_addr, uint16_t port)
{
	uint8_t res=1;
	uint16_t len=0;
	ip_pkt_ptr *ip_pkt = (void*)(frame->data);
	tcp_pkt_ptr *tcp_pkt = (void*)(ip_pkt->data);
	
	tcpprop.seq_num = tcp_pkt->num_ask;
	tcpprop.ack_num = be32todword(be32todword(tcp_pkt->bt_num_seg) + 1);
	len = sizeof(tcp_pkt_ptr);
	tcp_header_prepare(tcp_pkt, port, TCP_ACK, len);
	
	len+=sizeof(ip_pkt_ptr);
	ip_header_prepare(ip_pkt, ip_addr, IP_TCP, len);
	
	//Заполним заголовок Ethernet
	frame->type = ETH_IP;
	len+=sizeof(enc28j60_frame_ptr);
	eth_send(frame,len);
	
	if(tcp_stat == TCP_DISCONNECTED) return 1;
	
	tcp_pkt->fl = TCP_FIN|TCP_ACK;
	len = sizeof(tcp_pkt_ptr);
	tcp_pkt->cs = be16toword(IP_TCP + len);
	tcp_pkt->cs=checksum((uint8_t*)tcp_pkt-8, len+8);
	
	len+=sizeof(ip_pkt_ptr);
	
	memcpy(frame->addr_src,frame->addr_dest,6);
	memcpy(frame->addr_dest,macaddr,6);
	len+=sizeof(enc28j60_frame_ptr);
	eth_send(frame,len);
	
	tcp_stat = TCP_DISCONNECTED;

	return res;
}

/*Подтверждение на пакет данных и ответ при условии*/
uint8_t tcp_send_data(enc28j60_frame_ptr *frame, uint8_t *ip_addr, uint16_t port)
{
  uint8_t res=1;
  uint16_t len=0;
  uint16_t sz_data=0;
  tcp_stat = TCP_CONNECTED;
  ip_pkt_ptr *ip_pkt = (void*)(frame->data);
  tcp_pkt_ptr *tcp_pkt = (void*)(ip_pkt->data);
	
  sz_data = be16toword(ip_pkt->len)-20-(tcp_pkt->len_hdr>>2);
  tcpprop.seq_num = tcp_pkt->num_ask;
  tcpprop.ack_num = be32todword(be32todword(tcp_pkt->bt_num_seg) + sz_data);
  len = sizeof(tcp_pkt_ptr);	
  //отправим подтверждение на пакет данных
  tcp_header_prepare(tcp_pkt, port, TCP_ACK, len);
  
	len+=sizeof(ip_pkt_ptr);
  ip_header_prepare(ip_pkt, ip_addr, IP_TCP, len);
  
	//Заполним заголовок Ethernet
  frame->type = ETH_IP;
	len+=sizeof(enc28j60_frame_ptr);
  eth_send(frame,len);
  
  //Если пришло "Hello!!!", то отправим ответ
  if (!strcmp((char*)tcp_pkt->data,"Hello!!!"))
  {
    strcpy((char*)tcp_pkt->data,"Hello to TCP Client2!!!\r\n");
    tcp_pkt->fl = TCP_ACK|TCP_PSH;
    len = sizeof(tcp_pkt_ptr);
    len+=strlen((char*)tcp_pkt->data);
    tcp_pkt->cs = be16toword(IP_TCP + len);
    tcp_pkt->cs=checksum((uint8_t*)tcp_pkt-8, len+8);
    
	  len+=sizeof(ip_pkt_ptr);
    ip_pkt->len=be16toword(len);
    ip_pkt->cs = 0;
    ip_pkt->cs = checksum((void*)ip_pkt,sizeof(ip_pkt_ptr));
    
		//Заполним заголовок Ethernet
		memcpy(frame->addr_src,frame->addr_dest,6);
		memcpy(frame->addr_dest,macaddr,6);
		len+=sizeof(enc28j60_frame_ptr);
		eth_send(frame,len);	  
  }

  return res;
}

//uint8_t tcp_send(uint8_t *ip_addr, uint16_t port, uint8_t op)
//{
//	uint8_t res=0;
//	uint16_t len=0;
//	
//	static uint32_t num_seg=0;
//	uint16_t sz_data=0;	
//	
//	enc28j60_frame_ptr *frame=(void*) net_buf;
//	ip_pkt_ptr *ip_pkt = (void*)(frame->data);
//	tcp_pkt_ptr *tcp_pkt = (void*)(ip_pkt->data);
//	
//	if (op==TCP_OP_SYNACK)
//	{
//		//Заполним заголовок пакета TCP
//		tcp_pkt->port_dst = be16toword(port);
//		tcp_pkt->port_src = be16toword(LOCAL_PORT_TCP);
//		num_seg = tcp_pkt->num_ask;
//		tcp_pkt->num_ask = be32todword(be32todword(tcp_pkt->bt_num_seg) + 1);
//		tcp_pkt->bt_num_seg = be32todword((uint32_t)0x4455);
//		tcp_pkt->fl = TCP_SYN | TCP_ACK;
//		tcp_pkt->size_wnd = be16toword(8192);
//		tcp_pkt->urg_ptr = 0;
//		len = sizeof(tcp_pkt_ptr)+4;
//		tcp_pkt->len_hdr = len << 2;
//		tcp_pkt->data[0]=2;//Maximum Segment Size (2)
//		tcp_pkt->data[1]=4;//Length
//		tcp_pkt->data[2]=0x05;
//		tcp_pkt->data[3]=0x82;
//		tcp_pkt->cs = be16toword(IP_TCP + len);
//		tcp_pkt->cs=checksum((uint8_t*)tcp_pkt-8, len+8);	
//	
//		//Заполним заголовок пакета IP
//		sprintf(str1,"len:%d\r\n", len);
//		uart1_send_buf((uint8_t*)str1, strlen(str1));
//		
//		len+=sizeof(ip_pkt_ptr);
//		sprintf(str1,"len:%d\r\n", len);
//		uart1_send_buf((uint8_t*)str1, strlen(str1));
//		
//		ip_pkt->len=be16toword(len);
//		ip_pkt->id = 0;
//		ip_pkt->ts = 0;
//		ip_pkt->verlen = 0x45;
//		ip_pkt->fl_frg_of=0;
//		ip_pkt->ttl=128;
//		ip_pkt->cs = 0;
//		ip_pkt->prt=IP_TCP;
//		memcpy(ip_pkt->ipaddr_dst,ip_addr,4);
//		memcpy(ip_pkt->ipaddr_src,ipaddr,4);
//		ip_pkt->cs = checksum((void*)ip_pkt,sizeof(ip_pkt_ptr));
//		
//		memcpy(frame->addr_dest,frame->addr_src,6);
//		memcpy(frame->addr_src,macaddr,6);
//		frame->type = ETH_IP;
//		len+=sizeof(enc28j60_frame_ptr);
//		enc28j60_packetSend((void*)frame,len);
//		
//		sprintf(str1,"len:%d\r\n", len);
//		uart1_send_buf((uint8_t*)str1, strlen(str1));
//		uart1_send_buf((uint8_t*)"SYN ACK\r\n",9);
//	
//	} else if (op==TCP_OP_ACK_OF_FIN) {
//		//Заполним заголовок пакета TCP
//		tcp_pkt->port_dst = be16toword(port);
//		tcp_pkt->port_src = be16toword(LOCAL_PORT_TCP);
//		num_seg = tcp_pkt->num_ask;
//		tcp_pkt->num_ask = be32todword(be32todword(tcp_pkt->bt_num_seg) + 1);
//		tcp_pkt->bt_num_seg = num_seg;
//		tcp_pkt->fl = TCP_ACK;
//		tcp_pkt->size_wnd = be16toword(8192);
//		tcp_pkt->urg_ptr = 0;
//		len = sizeof(tcp_pkt_ptr);
//		tcp_pkt->len_hdr = len << 2;		
//		tcp_pkt->cs = be16toword(IP_TCP + len);
//		tcp_pkt->cs=checksum((uint8_t*)tcp_pkt-8, len+8);
//		
//		//Заполним заголовок пакета IP
//		sprintf(str1,"len:%d\r\n", len);
//		uart1_send_buf((uint8_t*)str1, strlen(str1));
//		
//		len+=sizeof(ip_pkt_ptr);
//		sprintf(str1,"len:%d\r\n", len);
//		uart1_send_buf((uint8_t*)str1, strlen(str1));
//		
//		ip_pkt->len=be16toword(len);
//		ip_pkt->id = 0;
//		ip_pkt->ts = 0;
//		ip_pkt->verlen = 0x45;
//		ip_pkt->fl_frg_of=0;
//		ip_pkt->ttl=128;
//		ip_pkt->cs = 0;
//		ip_pkt->prt=IP_TCP;
//		memcpy(ip_pkt->ipaddr_dst,ip_addr,4);
//		memcpy(ip_pkt->ipaddr_src,ipaddr,4);
//		ip_pkt->cs = checksum((void*)ip_pkt,sizeof(ip_pkt_ptr));
//		
//		memcpy(frame->addr_dest,frame->addr_src,6);
//		memcpy(frame->addr_src,macaddr,6);
//		frame->type = ETH_IP;
//		len+=sizeof(enc28j60_frame_ptr);
//		enc28j60_packetSend((void*)frame,len);
//		
//		sprintf(str1,"len:%d\r\n", len);
//		uart1_send_buf((uint8_t*)str1, strlen(str1));
//		uart1_send_buf((uint8_t*)"ACK OF FIN\r\n",9);
//		
//		tcp_pkt->fl = TCP_FIN|TCP_ACK;
//		len = sizeof(tcp_pkt_ptr);
//		tcp_pkt->cs = be16toword(IP_TCP + len);
//		tcp_pkt->cs=checksum((uint8_t*)tcp_pkt-8, len+8);
//		len+=sizeof(ip_pkt_ptr);
//		len+=sizeof(enc28j60_frame_ptr);
//		enc28j60_packetSend((void*)frame,len);		
//	
//	} else if (op==TCP_OP_ACK_OF_DATA) {
//		//Заполним заголовок пакета TCP
//		sz_data = be16toword(ip_pkt->len)-20-(tcp_pkt->len_hdr>>2);
//		
//		tcp_pkt->port_dst = be16toword(port);
//		tcp_pkt->port_src = be16toword(LOCAL_PORT_TCP);
//		num_seg = tcp_pkt->num_ask;																							// забыл эту строку
//		tcp_pkt->num_ask = be32todword(be32todword(tcp_pkt->bt_num_seg) + sz_data);
//		tcp_pkt->bt_num_seg = num_seg;
//		tcp_pkt->fl = TCP_ACK;
//		tcp_pkt->size_wnd = be16toword(8192);
//		tcp_pkt->urg_ptr = 0;
//		len = sizeof(tcp_pkt_ptr);
//		tcp_pkt->len_hdr = len << 2;		
//		tcp_pkt->cs = be16toword(IP_TCP + len);
//		tcp_pkt->cs=checksum((uint8_t*)tcp_pkt-8, len+8);	
//	
//		//Заполним заголовок пакета IP
//		sprintf(str1,"len:%d\r\n", len);
//		uart1_send_buf((uint8_t*)str1, strlen(str1));
//		
//		len+=sizeof(ip_pkt_ptr);
//		sprintf(str1,"len:%d\r\n", len);
//		uart1_send_buf((uint8_t*)str1, strlen(str1));
//		
//		ip_pkt->len=be16toword(len);
//		ip_pkt->id = 0;
//		ip_pkt->ts = 0;
//		ip_pkt->verlen = 0x45;
//		ip_pkt->fl_frg_of=0;
//		ip_pkt->ttl=128;
//		ip_pkt->cs = 0;
//		ip_pkt->prt=IP_TCP;
//		memcpy(ip_pkt->ipaddr_dst,ip_addr,4);
//		memcpy(ip_pkt->ipaddr_src,ipaddr,4);
//		ip_pkt->cs = checksum((void*)ip_pkt,sizeof(ip_pkt_ptr));
//		
//		memcpy(frame->addr_dest,frame->addr_src,6);
//		memcpy(frame->addr_src,macaddr,6);
//		frame->type = ETH_IP;
//		len+=sizeof(enc28j60_frame_ptr);
//		enc28j60_packetSend((void*)frame,len);
//		
//		if (!strcmp((char*)tcp_pkt->data,"Hello!!!"))
//		{
//			strcpy((char*)tcp_pkt->data,"Hello to TCP Client!!!\r\n");
//			tcp_pkt->fl = TCP_ACK|TCP_PSH;
//			sprintf(str1,"hdr_len:%d\r\n",sizeof(tcp_pkt_ptr));
//			uart1_send_buf((uint8_t*)str1,strlen(str1));
//			len = sizeof(tcp_pkt_ptr);
//			tcp_pkt->len_hdr = len << 2;
//			len+=strlen((char*)tcp_pkt->data);
//			tcp_pkt->cs = be16toword(IP_TCP + len);
//			tcp_pkt->cs=checksum((uint8_t*)tcp_pkt-8, len+8);
//			//Заполним заголовок пакета IP
//			len+=sizeof(ip_pkt_ptr);
//			ip_pkt->len=be16toword(len);
//			ip_pkt->cs = 0;
//			ip_pkt->cs = checksum((void*)ip_pkt,sizeof(ip_pkt_ptr));
//			len+=sizeof(enc28j60_frame_ptr);
//			enc28j60_packetSend((void*)frame,len);
//		}
//	}
//	
//	res = 1;
//	return res;
//}

//uint8_t tcp_send(uint8_t *ip_addr, uint16_t port, uint8_t op)
//{
//	uint16_t sz_data=0;
//  uint8_t res=0;
//  uint16_t len=0;
//	static uint32_t num_seg=0;
//	enc28j60_frame_ptr *frame=(void*) net_buf;
//	ip_pkt_ptr *ip_pkt = (void*)(frame->data);
//	tcp_pkt_ptr *tcp_pkt = (void*)(ip_pkt->data);
//	if (op==TCP_OP_SYNACK)
//	{
//		//Заполним заголовок пакета TCP
//		tcp_pkt->port_dst = be16toword(port);
//		tcp_pkt->port_src = be16toword(LOCAL_PORT_TCP);
//		tcp_pkt->num_ask = be32todword(be32todword(tcp_pkt->bt_num_seg) + 1);
//		tcp_pkt->bt_num_seg = rand();
//		tcp_pkt->fl = TCP_SYN | TCP_ACK;
//		tcp_pkt->size_wnd = be16toword(8192);
//		tcp_pkt->urg_ptr = 0;
//		len = sizeof(tcp_pkt_ptr)+4;
//		tcp_pkt->len_hdr = len << 2;
//		tcp_pkt->data[0]=2;//Maximum Segment Size (2)
//		tcp_pkt->data[1]=4;//Length
//		tcp_pkt->data[2]=0x05;
//		tcp_pkt->data[3]=0x82;
//		tcp_pkt->cs = be16toword(IP_TCP + len);
//		tcp_pkt->cs=checksum((uint8_t*)tcp_pkt-8, len+8);
//		//Заполним заголовок пакета IP
//		sprintf(str1,"len:%d\r\n", len);
////		HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
//		len+=sizeof(ip_pkt_ptr);
//		sprintf(str1,"len:%d\r\n", len);
////		HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
//		ip_pkt->len=be16toword(len);
//		ip_pkt->id = 0;
//		ip_pkt->ts = 0;
//		ip_pkt->verlen = 0x45;
//		ip_pkt->fl_frg_of=0;
//		ip_pkt->ttl=128;
//		ip_pkt->cs = 0;
//		ip_pkt->prt=IP_TCP;
//		memcpy(ip_pkt->ipaddr_dst,ip_addr,4);
//		memcpy(ip_pkt->ipaddr_src,ipaddr,4);
//		ip_pkt->cs = checksum((void*)ip_pkt,sizeof(ip_pkt_ptr));
//		//Заполним заголовок Ethernet
//		memcpy(frame->addr_dest,frame->addr_src,6);
//		memcpy(frame->addr_src,macaddr,6);
//		frame->type=ETH_IP;
//		len+=sizeof(enc28j60_frame_ptr);
//		enc28j60_packetSend((void*)frame,len);
//		sprintf(str1,"len:%d\r\n", len);
////		HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
////		HAL_UART_Transmit(&huart1,(uint8_t*)"SYN ACK\r\n",9,0x1000);
//	}
//	else if (op==TCP_OP_ACK_OF_FIN)
//	{
//		//Заполним заголовок пакета TCP
//		tcp_pkt->port_dst = be16toword(port);
//		tcp_pkt->port_src = be16toword(LOCAL_PORT_TCP);
//		num_seg = tcp_pkt->num_ask;
//		tcp_pkt->num_ask = be32todword(be32todword(tcp_pkt->bt_num_seg) + 1);
//		//передадим 0 в USART, иначе подвисает код
//		uart1_send_buf((uint8_t*)0,1);
//		tcp_pkt->bt_num_seg = num_seg;
//		tcp_pkt->fl = TCP_ACK;
//		tcp_pkt->size_wnd = be16toword(8192);
//		tcp_pkt->urg_ptr = 0;
//		len = sizeof(tcp_pkt_ptr);
//		tcp_pkt->len_hdr = len << 2;
//		tcp_pkt->cs = be16toword(IP_TCP + len);
//		tcp_pkt->cs=checksum((uint8_t*)tcp_pkt-8, len+8);
//		//Заполним заголовок пакета IP
//		sprintf(str1,"len:%d\r\n", len);
////		HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
//		len+=sizeof(ip_pkt_ptr);
//		sprintf(str1,"len:%d\r\n", len);
////		HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
//		ip_pkt->len=be16toword(len);
//		ip_pkt->id = 0;
//		ip_pkt->ts = 0;
//		ip_pkt->verlen = 0x45;
//		ip_pkt->fl_frg_of=0;
//		ip_pkt->ttl=128;
//		ip_pkt->cs = 0;
//		ip_pkt->prt=IP_TCP;
//		memcpy(ip_pkt->ipaddr_dst,ip_addr,4);
//		memcpy(ip_pkt->ipaddr_src,ipaddr,4);
//		ip_pkt->cs = checksum((void*)ip_pkt,sizeof(ip_pkt_ptr));
//		//Заполним заголовок Ethernet
//		memcpy(frame->addr_dest,frame->addr_src,6);
//		memcpy(frame->addr_src,macaddr,6);
//		frame->type=ETH_IP;
//		len+=sizeof(enc28j60_frame_ptr);
//		enc28j60_packetSend((void*)frame,len);
////		HAL_UART_Transmit(&huart1,(uint8_t*)"ACK OF FIN\r\n",12,0x1000);
//		tcp_pkt->fl = TCP_FIN|TCP_ACK;
//    len = sizeof(tcp_pkt_ptr);
//    tcp_pkt->cs = 0;
//    tcp_pkt->cs=checksum((uint8_t*)tcp_pkt-8, len+8);
//    len+=sizeof(ip_pkt_ptr);
//    len+=sizeof(enc28j60_frame_ptr);
//    enc28j60_packetSend((void*)frame,len);
//	}
//	else if (op==TCP_OP_ACK_OF_DATA)
//	{
//		//Заполним заголовок пакета TCP
//		sz_data = be16toword(ip_pkt->len)-20-(tcp_pkt->len_hdr>>2);
//		tcp_pkt->port_dst = be16toword(port);
//		tcp_pkt->port_src = be16toword(LOCAL_PORT_TCP);
//		num_seg = tcp_pkt->num_ask;
//		tcp_pkt->num_ask = be32todword(be32todword(tcp_pkt->bt_num_seg) + sz_data);
//		sprintf(str1,"sz_data:%u\r\n", sz_data);
////		HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
//		tcp_pkt->bt_num_seg = num_seg;
//		tcp_pkt->fl = TCP_ACK;
//		tcp_pkt->size_wnd = be16toword(8192);
//		tcp_pkt->urg_ptr = 0;
//		len = sizeof(tcp_pkt_ptr);
//		tcp_pkt->len_hdr = len << 2;
//		tcp_pkt->cs = be16toword(IP_TCP + len);
//		tcp_pkt->cs=checksum((uint8_t*)tcp_pkt-8, len+8);
//		//Заполним заголовок пакета IP
//		sprintf(str1,"len:%d\r\n", len);
////		HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
//		len+=sizeof(ip_pkt_ptr);
//		sprintf(str1,"len:%d\r\n", len);
////		HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
//		ip_pkt->len=be16toword(len);
//		ip_pkt->id = 0;
//		ip_pkt->ts = 0;
//		ip_pkt->verlen = 0x45;
//		ip_pkt->fl_frg_of=0;
//		ip_pkt->ttl=128;
//		ip_pkt->cs = 0;
//		ip_pkt->prt=IP_TCP;
//		memcpy(ip_pkt->ipaddr_dst,ip_addr,4);
//		memcpy(ip_pkt->ipaddr_src,ipaddr,4);
//		ip_pkt->cs = checksum((void*)ip_pkt,sizeof(ip_pkt_ptr));
//		//Заполним заголовок Ethernet
//		memcpy(frame->addr_dest,frame->addr_src,6);
//		memcpy(frame->addr_src,macaddr,6);
//		frame->type=ETH_IP;
//		len+=sizeof(enc28j60_frame_ptr);
//		enc28j60_packetSend((void*)frame,len);
//		//Если пришло "Hello!!!", то отправим ответ
//		if (!strcmp((char*)tcp_pkt->data,"Hello!!!"))
//		{
//			strcpy((char*)tcp_pkt->data,"Hello to TCP Client!!!\r\n");
//			tcp_pkt->fl = TCP_ACK|TCP_PSH;
//			sprintf(str1,"hdr_len:%d\r\n",sizeof(tcp_pkt_ptr));
////			HAL_UART_Transmit(&huart1,(uint8_t*)str1,strlen(str1),0x1000);
//			len = sizeof(tcp_pkt_ptr);
//			tcp_pkt->len_hdr = len << 2;
//			len+=strlen((char*)tcp_pkt->data);
//			tcp_pkt->cs = be16toword(IP_TCP + len);
//			tcp_pkt->cs=checksum((uint8_t*)tcp_pkt-8, len+8);
//			//Заполним заголовок пакета IP
//			len+=sizeof(ip_pkt_ptr);
//			ip_pkt->len=be16toword(len);
//			ip_pkt->cs = 0;
//			ip_pkt->cs = checksum((void*)ip_pkt,sizeof(ip_pkt_ptr));
//			len+=sizeof(enc28j60_frame_ptr);
//			enc28j60_packetSend((void*)frame,len);
//		}
//	}
//  return res=1;
//}

/*Отправка однопакетного ответа HTTP*/

uint8_t tcp_send_http_one(enc28j60_frame_ptr *frame, uint8_t *ip_addr, uint16_t port)
{
  uint8_t res=1;
  uint16_t len=0;
  uint16_t sz_data=0;
  ip_pkt_ptr *ip_pkt = (void*)(frame->data);
  tcp_pkt_ptr *tcp_pkt = (void*)(ip_pkt->data);
	
	//Отправим сначала подтверждение на пакет запроса
	sz_data = be16toword(ip_pkt->len)-20-(tcp_pkt->len_hdr>>2);
	tcpprop.seq_num = tcp_pkt->num_ask;
	tcpprop.ack_num = be32todword(be32todword(tcp_pkt->bt_num_seg) + sz_data);
	len = sizeof(tcp_pkt_ptr);
	tcp_header_prepare(tcp_pkt, port, TCP_ACK, len);
	
	len+=sizeof(ip_pkt_ptr);
	ip_header_prepare(ip_pkt, ip_addr, IP_TCP, len);
	
	//Заполним заголовок Ethernet
	frame->type = ETH_IP;
	len+=sizeof(enc28j60_frame_ptr);
	eth_send(frame,len);
	
	//Отправляем страницу
	strcpy((char*)tcp_pkt->data,http_header);
	memcpy((void*)(tcp_pkt->data+strlen(http_header)),(void*)index_htm,sizeof(index_htm));
	
	len = sizeof(tcp_pkt_ptr);
	len+=tcpprop.data_size;
	tcp_pkt->fl = TCP_PSH|TCP_ACK;
	tcp_pkt->cs = be16toword(IP_TCP + len);
	tcp_pkt->cs=checksum((uint8_t*)tcp_pkt-8, len+8);
	
	len+=sizeof(ip_pkt_ptr);
	ip_pkt->len=be16toword(len);
	ip_pkt->cs = 0;
	ip_pkt->cs = checksum((void*)ip_pkt,sizeof(ip_pkt_ptr));
	
	//Заполним заголовок Ethernet
	memcpy(frame->addr_src,frame->addr_dest,6);
	memcpy(frame->addr_dest,macaddr,6);
	len+=sizeof(enc28j60_frame_ptr);
	eth_send(frame,len);
	
	tcpprop.data_stat=DATA_END;
  return res;
}

/*Разъединяемся после получения подтверждения на последний пакет данных*/
uint8_t tcp_send_http_dataend(enc28j60_frame_ptr *frame, uint8_t *ip_addr, uint16_t port)
{
  uint8_t res=1;
  uint16_t len=0;
  ip_pkt_ptr *ip_pkt = (void*)(frame->data);
  tcp_pkt_ptr *tcp_pkt = (void*)(ip_pkt->data);
  tcpprop.seq_num = tcp_pkt->num_ask;
  tcpprop.ack_num = tcp_pkt->bt_num_seg;
  len = sizeof(tcp_pkt_ptr);
  tcp_header_prepare(tcp_pkt, port, TCP_FIN|TCP_ACK, len);
  
	len+=sizeof(ip_pkt_ptr);
  ip_header_prepare(ip_pkt, ip_addr, IP_TCP, len);
  
	//Заполним заголовок Ethernet
  frame->type = ETH_IP;
	len+=sizeof(enc28j60_frame_ptr);
  eth_send(frame,len);
  
	tcpprop.data_stat=DATA_COMPLETED;
  tcp_stat = TCP_DISCONNECTED;
  return res;
}

uint8_t tcp_read (enc28j60_frame_ptr *frame, uint16_t len)
{	
	uint16_t len_data=0;
	uint16_t i=0;
	uint8_t res=0;
	
	ip_pkt_ptr *ip_pkt = (void*)(frame->data);	
	tcp_pkt_ptr *tcp_pkt = (void*)(ip_pkt->data);
	tcpprop.port_dst = be16toword(tcp_pkt->port_src);
	
	len_data = be16toword(ip_pkt->len)-20-(tcp_pkt->len_hdr>>2);
	
	sprintf(str1,"%d.%d.%d.%d-%d.%d.%d.%d %d tcp\r\n",
	ip_pkt->ipaddr_src[0],ip_pkt->ipaddr_src[1],ip_pkt->ipaddr_src[2],ip_pkt->ipaddr_src[3],
	ip_pkt->ipaddr_dst[0],ip_pkt->ipaddr_dst[1],ip_pkt->ipaddr_dst[2],ip_pkt->ipaddr_dst[3], len_data);
	
//	sprintf(str1,"%02X:%02X:%02X:%02X:%02X:%02X-%02X:%02X:%02X:%02X:%02X:%02X; %d tcp\r\n",
//	frame->addr_src[0],frame->addr_src[1],frame->addr_src[2],
//	frame->addr_src[3],frame->addr_src[4],frame->addr_src[5],
//	frame->addr_dest[0],frame->addr_dest[1],frame->addr_dest[2],
//	frame->addr_dest[3],frame->addr_dest[4],frame->addr_dest[5],len);
	uart1_send_buf((uint8_t*)str1, strlen(str1));
	
	if (len_data) {
		for (i=0;i<len_data;i++) {
			uart1_send_buf(tcp_pkt->data+i,1);
		}
		uart1_send_buf((uint8_t*)"\r\n", 2);		
		if (tcp_pkt->fl&TCP_ACK){
			//res = tcp_send(ip_pkt->ipaddr_src, be16toword(tcp_pkt->port_src), TCP_OP_ACK_OF_DATA);
			
			//Если строка "GET / ", то значит это запрос HTTP главной страницы, пока будем работать только с одной
			if (strncmp((char*)tcp_pkt->data,"GET / ", 6) == 0)
			{
				tcpprop.data_size = strlen(http_header) + sizeof(index_htm);
				tcpprop.cnt_data_part = tcpprop.data_size / tcp_mss + 1;
				tcpprop.last_data_part_size = tcpprop.data_size % tcp_mss;
				sprintf(str1,"data size:%lu; cnt data part:%u; last_data_part_size:%urnport dst:%urn",
				(unsigned long)tcpprop.data_size, tcpprop.cnt_data_part, tcpprop.last_data_part_size,tcpprop.port_dst);
				uart1_send_buf((uint8_t*)str1,strlen(str1));
				
				if (tcpprop.cnt_data_part==1){
					tcpprop.data_stat = DATA_ONE;
				}
				else if (tcpprop.cnt_data_part>1)	{
					tcpprop.data_stat = DATA_FIRST;
				}
				
				if(tcpprop.data_stat==DATA_ONE)
				{
					res = tcp_send_http_one(frame, ip_pkt->ipaddr_src, tcpprop.port_dst);
				}
	
			}
			//Иначе обычные данные
			else {
				res = tcp_send_data(frame, ip_pkt->ipaddr_src, tcpprop.port_dst);
			}		
		}
	}

	if (tcp_pkt->fl == TCP_SYN)
	{
		sprintf(str1,"TCP_SYN_FL\r\n");
		uart1_send_buf((uint8_t*)str1, strlen(str1));
		//res = tcp_send(ip_pkt->ipaddr_src, be16toword(tcp_pkt->port_src), TCP_OP_SYNACK);
		res = tcp_send_synack(frame, ip_pkt->ipaddr_src, tcpprop.port_dst);
	}		
	else if (tcp_pkt->fl == (TCP_FIN|TCP_ACK)) {
		sprintf(str1,"TCP_FIN|TCP_ACK\r\n");
		uart1_send_buf((uint8_t*)str1, strlen(str1));
		//res = tcp_send(ip_pkt->ipaddr_src, be16toword(tcp_pkt->port_src), TCP_OP_ACK_OF_FIN);
		res = tcp_send_finack(frame, ip_pkt->ipaddr_src, tcpprop.port_dst);
	} else if (tcp_pkt->fl == (TCP_PSH|TCP_ACK)) {
		//Если данных нет
		if(!len_data) {
			//res = tcp_send(ip_pkt->ipaddr_src, be16toword(tcp_pkt->port_src), TCP_OP_ACK_OF_FIN);
			res = tcp_send_finack(frame, ip_pkt->ipaddr_src, tcpprop.port_dst);
		}
	}
	else if (tcp_pkt->fl == TCP_ACK) {
		//sprintf(str1,"TCP_ACK\r\n");
		//uart1_send_buf((uint8_t*)str1, strlen(str1));
		//uart1_send_buf((uint8_t*)"TCP_ACK\r\n",7);
		if (tcpprop.data_stat==DATA_END)
		{
			res = tcp_send_http_dataend(frame, ip_pkt->ipaddr_src, tcpprop.port_dst);
		}
	}	

  return res;
}
