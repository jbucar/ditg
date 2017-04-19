/*
 *   Component of the D-ITG 2.8.1 (r1023) platform (http://traffic.comics.unina.it/software/ITG)
 *
 *   Copyright     : (C) 2004-2013 by Alessio Botta, Walter de Donato, Alberto Dainotti,
 *                                      Stefano Avallone, Antonio Pescape' (PI)
 *                                      COMICS (COMputer for Interaction and CommunicationS) Group
 *                                      Department of Electrical Engineering and Information Technologies
 *                                      University of Napoli "Federico II".
 *   email         : a.botta@unina.it, walter.dedonato@unina.it, alberto@unina.it,
 *                   stavallo@unina.it, pescape@unina.it
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 * 		     
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *				     
 *   For commercial use please refer to D-ITG Professional.
 */



#include "../libITG/ITGapi.h"
#include "thread.h"
#include "debug.h"

#include <string.h>		
#include <stdlib.h>		
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <sys/types.h>		
#include <signal.h>
#include <errno.h>
#include <math.h>

using namespace std;

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

#include <io.h>
#include <stdint.h>
typedef int pid_t;
typedef int socklen_t;
typedef int uid_t;
typedef u_long in_addr_t;
typedef u_short in_port_t;
typedef HANDLE pthread_t;
#define MSG_DONTWAIT 0x40
#ifndef IPV6_V6ONLY
# define IPV6_V6ONLY 27
#endif
#endif

#ifdef UNIX
#include <netinet/in.h>		
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>		
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/mman.h>		
#include <sched.h>		

#ifdef SCTP
#include <netinet/sctp.h> 
typedef struct {
	union {
		unsigned int numStreams;
		unsigned int port;
	};
	unsigned int parsedStreams;
	unsigned int busyStreams;
	int sock;
} sctpSession;
#endif

#ifdef DCCP
#define DCCP_SOCKOPT_PACKET_SIZE 1
#define DCCP_SOCKOPT_SERVICE 2
#ifndef SOCK_DCCP
#define SOCK_DCCP 6
#endif
#define TCP_CONGESTION	13
#define DCCP_SOCKOPT_CCID_RX_INFO 128
#define DCCP_SOCKOPT_CCID_TX_INFO 192
#define SOL_DCCP 269
#endif

typedef unsigned short USHORT;
typedef char UCHAR;
typedef unsigned char BYTE;
typedef long int ULONG;
#define INFINITE 60000
#define INVALID_HANDLE_VALUE -1
#ifdef BSD
#include <netinet/in_systm.h>
#define s6_addr   __u6_addr.__u6_addr8
#define s6_addr8  __u6_addr.__u6_addr8
#define s6_addr16 __u6_addr.__u6_addr16
#define s6_addr32 __u6_addr.__u6_addr32
#endif
#endif

#if defined WIN32 || defined BSD
#define SOL_IP   IPPROTO_IP
#define SOL_IPV6 IPPROTO_IPV6
#endif

#define MAX_L4_PORTS 65536

#define	MAX(a,b)	((a) > (b) ? (a) : (b)) 	

extern const char *meters[];
extern const char *l7Protocols[];
extern const char *l4Protocols[];
extern const int StandardMinPayloadSize;	
extern const int ShortMinPayloadSize; 		
extern const int NoneMinPayloadSize; 		


#define LX_ERROR_BYTE	255

#define LX_PROTO_NONE	0


#define L3_PROTO_IPv4	4
#define L3_PROTO_IPv6	6


#define L4_PROTO_TCP	1
#define L4_PROTO_UDP	2
#define L4_PROTO_ICMP	3
#define L4_PROTO_SCTP   4
#define L4_PROTO_DCCP	5


#define L7_PROTO_TELNET		1
#define L7_PROTO_VOIP		2
#define L7_PROTO_DNS		3
#define L7_PROTO_AOM		4
#define L7_PROTO_CSATTIVA	5
#define L7_PROTO_CSINATTIVA	6
#define L7_PROTO_QUAKE		7


#define PL_STANDARD 0 
#define PL_SHORT 1 
#define PL_NONE 2





#define LOG_CONNECT 1


#define METER_OWDM    1
#define METER_RTTM    2




#define NO_TERMINATE 0

#define TERMINATE 1

#define ERROR_TERMINATE 2




#define SENDER	0

#define RECEIVER	1



#define TSP_CONNECT			1

#define TSP_ACK_CONNECT			2

#define TSP_SEND_FLOW			3

#define TSP_CLOSED_FLOW			4

#define TSP_ACK_SEND_FLOW		5

#define TSP_ACK_CLOSED_FLOW		6

#define TSP_ACK_RELEASE			7

#define TSP_DISCOVERY			8

#define TSP_ACK_DISCOVERY		9

#define TSP_CRYPTO_RSA			10

#define TSP_RELEASE			11

#define TSP_SEND_FLOW_LOG		12

#define TSP_ACK_SEND_FLOW_LOG		13

#define TSP_ERR_MSG_1			14

#define TSP_ERR_MSG_2			15

#define TSP_ERR_MSG_3			16

#define TSP_CLOSED_ERR			17

#define TSP_ACK_SEND_NAME_LOG	18

#define TSP_SEND_NAME_LOG		19

#define TSP_SENDER_DOWN			20

#define TSP_ERR_MSG_4			21

#define TSP_ERR_MSG_5			22






#ifdef WIN32

#define USER_ID()		1
#endif

#ifdef UNIX

#define USER_ID()		getuid()
#endif


#define SET_PORT(host,port) \
	if (host->ai_family == PF_INET)  \
		((struct sockaddr_in*)(host->ai_addr))->sin_port=port;  \
	else if (host->ai_family == PF_INET6)  \
		((struct sockaddr_in6*)(host->ai_addr))->sin6_port=port;

#define GET_PORT(host,port) \
	if (host->ai_family == PF_INET)  \
		port=((struct sockaddr_in*)(host->ai_addr))->sin_port;  \
	else if (host->ai_family == PF_INET6)  \
		port=((struct sockaddr_in6*)(host->ai_addr))->sin6_port;


#ifdef WIN32
# ifndef IN6_ARE_ADDR_EQUAL
#  define IN6_ARE_ADDR_EQUAL(a,b) IN6_ADDR_EQUAL(a,b)
# endif
# define ARE_INET_ADDR_EQUAL(a,b) \
  ( (a->ai_family == b->ai_family)  &&    \
    ( a->ai_family == PF_INET    ?        \
      ((struct sockaddr_in*)(a->ai_addr))->sin_addr.s_addr == ((struct sockaddr_in*)(b->ai_addr))->sin_addr.s_addr  :    \
      IN6_ARE_ADDR_EQUAL(&(((struct sockaddr_in6*)(a->ai_addr))->sin6_addr),&(((struct sockaddr_in6*)(b->ai_addr))->sin6_addr))  ))
#endif

#ifdef UNIX
#ifdef BSD
#define ARE_INET_ADDR_EQUAL(a,b) \
  ( (a->ai_family == b->ai_family)  &&    \
    ( a->ai_family == PF_INET    ?        \
      ((struct sockaddr_in*)(a->ai_addr))->sin_addr.s_addr == ((struct sockaddr_in*)(b->ai_addr))->sin_addr.s_addr  :    \
	IN6_ARE_ADDR_EQUAL(&((struct sockaddr_in6*)(a->ai_addr))->sin6_addr,&((struct sockaddr_in6*)(b->ai_addr))->sin6_addr)  ))
#else
#define ARE_INET_ADDR_EQUAL(a,b) \
  ( (a->ai_family == b->ai_family)  &&    \
    ( a->ai_family == PF_INET    ?        \
      ((struct sockaddr_in*)(a->ai_addr))->sin_addr.s_addr == ((struct sockaddr_in*)(b->ai_addr))->sin_addr.s_addr  :    \
      IN6_ARE_ADDR_EQUAL(((struct sockaddr_in6*)(a->ai_addr))->sin6_addr.s6_addr32,((struct sockaddr_in6*)(b->ai_addr))->sin6_addr.s6_addr32)  ))
#endif
#endif

#ifdef UNIX

#define INET_NTOP(host,dst,cnt)  \
	if (host->ai_family == PF_INET)  \
		inet_ntop(host->ai_family,&(((struct sockaddr_in*)host->ai_addr)->sin_addr),dst,cnt);  \
	else if (host->ai_family == PF_INET6)  \
		inet_ntop(host->ai_family,&(((struct sockaddr_in6*)host->ai_addr)->sin6_addr),dst,cnt);
#endif



#define DEFAULT_PORT_SIGNALING 		9000	
#define DEFAULT_PORT 			8999	
#define DEFAULT_PORT_SENDER 		8997	
#define DEFAULT_LOG_PORT 		9002	
#define DEFAULT_LOG_PORT_SIGNALING 	9001	
#define DEFAULT_PORT_SENDER_MANAGER 8998	

#define TIME_OUT 			100
#define DIM_LOG_FILE 			200
#define DIM_PAYLOAD_FILE		200 	
#define DIM_NAME_SERIAL_INTERFACE 	10

extern uint16_t logbuffer_size;			

#define MAX_PAYLOAD_SIZE 		64000	
#define MAX_NUM_THREAD 			400
#define MAX_FLOW_LINE_SIZE		1460	
#define MAX_MSG_SIZE 			100	
#define DEFAULT_PROTOCOL_TX_LOG		L4_PROTO_UDP
#define DEFAULT_PROTOCOL_TX_LOG_OPZ	L4_PROTO_UDP

#define DEFAULT_DEST_IP "127.0.0.1"

extern char programName[100];
extern char DEFAULT_LOG_IP[10];



struct signaling {
	BYTE protocol;
	bool stop;
	char pad_1; 
	char pad_2; 
	char logFile[DIM_LOG_FILE];
};



struct info {
  uint32_t flowId;
  uint32_t seqNum;
  uint32_t srcPort;	
  uint32_t destPort;
  uint32_t txTime1;
  uint32_t txTime2;
  uint32_t rxTime1;
  uint32_t rxTime2;
  uint32_t size;
  char srcAddr[INET6_ADDRSTRLEN];
  char destAddr[INET6_ADDRSTRLEN];
  double txTime3;
  double rxTime3;
};


struct icmpv6 {
	BYTE icmp_type;
	BYTE icmp_code;
	USHORT icmp_cksum;
};



struct icmp {
	BYTE icmp_type;
	BYTE icmp_code;
	USHORT icmp_cksum;
	USHORT icmp_id;
	USHORT icmp_seq;

};

#ifdef BSD
struct iphdr {
	unsigned char ihl:4;	
	unsigned char version:4;	
	unsigned char tos;	
	unsigned short tot_len;	
	unsigned short id;	
	unsigned short frag_off;	
	unsigned char ttl;
	unsigned char protocol;	
	unsigned short check;	
	unsigned int saddr;
	unsigned int daddr;
};
#endif

#ifdef WIN32

struct iphdr {
	unsigned char ihl:4;	
	unsigned char version:4;	
	unsigned char tos;	
	unsigned short tot_len;	
	unsigned short id;	
	unsigned short frag_off;	
	unsigned char ttl;
	unsigned char protocol;	
	unsigned short check;	
	unsigned int saddr;
	unsigned int daddr;
};

int InitializeWinsock(WORD wVersionRequested);
void sleep(int tempo);
#endif

char *allowedLogFile(char logFile[DIM_LOG_FILE]);

void closeFileLog(ofstream * out);


USHORT checksum(USHORT * buffer, int size);


char *putValue(void* startPos,void* data,int size);

BYTE findMeter(char *s);
BYTE findL4Proto(char *s);
BYTE findL7Proto(char *s);

const char* invFindMeter(BYTE meter);
const char* invFindL4Proto(BYTE proto);
const char* invFindL7Proto(BYTE proto);




inline void getInfo(struct addrinfo *Host,int &tmpPort,	char *HelpAddress)
{
    
	GET_PORT(Host, tmpPort);

    
    getnameinfo(Host->ai_addr, Host->ai_addrlen, HelpAddress, INET6_ADDRSTRLEN, NULL, 0, NI_NUMERICHOST);
}



inline int writeInBufferStandard(info * infos,int FlowId, int ptrSeqNum, char *SrcHost,
    char *DestHost, in_port_t tmp_portSrc, in_port_t tmp_portDest ,long time1, long time2,
    long txTime_Usec,long rxTime_Usec, int size)
{

	
	infos->flowId = ntohl(FlowId); 

	
	infos->seqNum = ntohl(ptrSeqNum); 

	
	strcpy(infos->srcAddr, SrcHost);

   	
   	infos->srcPort = ntohs(tmp_portSrc);

	
	strcpy(infos->destAddr, DestHost);

	
	infos->destPort = ntohs(tmp_portDest);

	
	infos->txTime1 = time1 / 3600;
	infos->txTime2 = (time1 % 3600) / 60;
	infos->txTime3 = time1 % 60 + txTime_Usec / 1000000.0;

	
	infos->rxTime1 = time2 / 3600;
	infos->rxTime2 = (time2 % 3600) / 60;
	infos->rxTime3 = time2 % 60 + rxTime_Usec / 1000000.0;
	infos->size = size;

	PRINTD(2,"\nwriteInBufferStandard: flowId : %d\n",infos->flowId);
	PRINTD(2,"\tSequence Number : %d\n",infos->seqNum);
	PRINTD(2,"\tSrc : %16s/%hu  ",infos->srcAddr,infos->srcPort);
	PRINTD(2,"Dest : %16s/%hu  ",infos->destAddr,infos->destPort);
	PRINTD(2,"TxTime : %02u:%02u:%06lf ",(unsigned int)infos->txTime1,(unsigned int)infos->txTime2,infos->txTime3);
	PRINTD(2,"RxTime : %02u:%02u:%06lf ",(unsigned int)infos->rxTime1,(unsigned int)infos->rxTime2,infos->rxTime3);
	PRINTD(2,"Size : %5u\n\n",infos->size);
    return 0;
}

inline int writeInBufferShort(info * infos,int FlowId, int ptrSeqNum, char *SrcHost,
    char *DestHost, in_port_t tmp_portSrc, in_port_t tmp_portDest , long time2,
   long rxTime_Usec, int size)
{
	
	infos->flowId = ntohl(FlowId); 

	
	infos->seqNum = ntohl(ptrSeqNum); 
	
	strcpy(infos->srcAddr, SrcHost);

   	
   	infos->srcPort = ntohs(tmp_portSrc);

	
	strcpy(infos->destAddr, DestHost);

	
	infos->destPort = ntohs(tmp_portDest);

	infos->rxTime1 = time2 / 3600;
	infos->rxTime2 = (time2 % 3600) / 60;
	infos->rxTime3 = time2 % 60 + rxTime_Usec / 1000000.0;

	
	infos->txTime1 = time2 / 3600;
	infos->txTime2 = (time2 % 3600) / 60;
	infos->txTime3 = time2 % 60 + rxTime_Usec / 1000000.0;

	infos->size = size;

	PRINTD(2,"\nwriteInBufferShort: flowId : %d\n",infos->flowId);
	PRINTD(2,"\tSequence Number : %d\n",infos->seqNum);
	PRINTD(2,"\tSrc : %16s/%hu  ",infos->srcAddr,infos->srcPort);
	PRINTD(2,"Dest : %16s/%hu  ",infos->destAddr,infos->destPort);
	PRINTD(2,"TxTime : %u:%u:%lf ",(unsigned int)infos->txTime1,(unsigned int)infos->txTime2,infos->txTime3);
	PRINTD(2,"RxTime : %u:%u:%lf ",(unsigned int)infos->rxTime1,(unsigned int)infos->rxTime2,infos->rxTime3);
	PRINTD(2,"Size : %5u\n\n",infos->size);
     return 0;
}

inline int writeInBufferNone(info * infos,int FlowId, char *SrcHost,
    char *DestHost, in_port_t tmp_portSrc, in_port_t tmp_portDest , long time2,
   long rxTime_Usec, int size)
{
	
	
	infos->flowId = (unsigned int)FlowId; 

	
	infos->seqNum = 0;
	
	strcpy(infos->srcAddr, SrcHost);

   	
   	infos->srcPort = ntohs(tmp_portSrc);

	
	strcpy(infos->destAddr, DestHost);

	
	infos->destPort = ntohs(tmp_portDest);

	infos->rxTime1 = time2 / 3600;
	infos->rxTime2 = (time2 % 3600) / 60;
	infos->rxTime3 = time2 % 60 + rxTime_Usec / 1000000.0;

	
	infos->txTime1 = time2 / 3600;
	infos->txTime2 = (time2 % 3600) / 60;
	infos->txTime3 = time2 % 60 + rxTime_Usec / 1000000.0;

	infos->size = size;

	PRINTD(2,"\nwriteInBufferNone: flowId : %d\n",infos->flowId);
	PRINTD(2,"\tSequence Number : %d\n",infos->seqNum);
	PRINTD(2,"\tSrc : %16s/%hu  ",infos->srcAddr,infos->srcPort);
	PRINTD(2,"Dest : %16s/%hu  ",infos->destAddr,infos->destPort);
	PRINTD(2,"TxTime : %02u:%02u:%06lf ",(unsigned int)infos->txTime1,(unsigned int)infos->txTime2,infos->txTime3);
	PRINTD(2,"RxTime : %02u:%02u:%06lf ",(unsigned int)infos->rxTime1,(unsigned int)infos->rxTime2,infos->rxTime3);
	PRINTD(2,"Size : %5u\n\n",infos->size);
    return 0;
}

inline void infosHostToNet(struct info *infos){


	infos->flowId 	= htonl(infos->flowId);
	infos->seqNum 	= htonl(infos->seqNum);
	infos->srcPort 	= htonl(infos->srcPort);
	infos->destPort = htonl(infos->destPort);
	infos->size 	= htonl(infos->size);

	PRINTD(3,"infosHostToNet: Host Time (h:m:s): %02u:%02u:%06lf \n",(unsigned int)infos->rxTime1,(unsigned int)infos->rxTime2,infos->rxTime3);

	infos->rxTime1 = htonl(infos->rxTime1 * 3600 + infos->rxTime2 * 60 + (unsigned int)floor(infos->rxTime3));
	infos->rxTime2 = htonl((uint32_t)((infos->rxTime3 - (unsigned int)floor(infos->rxTime3)) * 1000000));
	infos->rxTime3 = 0;

	PRINTD(3,"infosHostToNet: Net Time   (s:us): %lu:%06lu \n",(unsigned long)ntohl(infos->rxTime1),(unsigned long)ntohl(infos->rxTime2));

	infos->txTime1 = htonl(infos->txTime1 * 3600 + infos->txTime2 * 60 + (unsigned int)floor(infos->txTime3));
	infos->txTime2 = htonl((uint32_t)((infos->txTime3 - (unsigned int)floor(infos->txTime3)) * 1000000));
	infos->txTime3 = 0;

}


inline int TCPrecvPacket(unsigned char *payload,int newSock, int preambleSize, int payloadLogType)
{

	
	
	unsigned char *ptrToSizeInPayload = NULL;

	if (payloadLogType == PL_STANDARD)
			ptrToSizeInPayload = payload + 4 * sizeof(uint32_t);
		else if (payloadLogType == PL_SHORT)
			ptrToSizeInPayload = payload + 2 * sizeof(uint32_t);
	
	

   	
	int sizePreamble =  preambleSize;

	
	unsigned char buffer[MAX_PAYLOAD_SIZE];

   	 

	

   	 
	int size = 0, size_p = 0, size_r = 0;

	size_r = recv(newSock, (char *) payload, sizePreamble, 0);

	
	if (size_r <= 0){
		return size_r;
	}

	PRINTDS(2,"TCPrecvPacket: Received TCP Preamble first time :%d \n",size_r);

	
#if DEBUG > 3
	printf("TCPrecvPacket: Preamble data (1): \n");
	for(int z=0;z<size_r;z++)
		printf("%.2X ",(int)payload[z]);
	printf("\n");
	fflush(stdout);
#endif
	

	
	if (size_r < sizePreamble) {
			
			int size_l = 0;
			
			int size_prec = size_r;
			
			unsigned char *ptr_payload;
			
			ptr_payload = payload + size_prec;

			
			do {
				
				size_l = recv(newSock, (char *) buffer, (sizePreamble - size_prec), 0);
				if (size_l <= 0){
				    return size_l;
				}

				PRINTD(2,"TCPrecvPacket: Received Other data of TCP Preamble %d\n",size_l);

				
#if DEBUG > 3
				printf("TCPrecvPacket: Preamble data (2): \n");
				for(int w=0;w<size_l;w++)
					printf("%.2X ",(int)buffer[w]);
				printf("\n");
				fflush(stdout);
#endif
				

				
				size_prec = size_l + size_prec;
				
				memcpy(ptr_payload, buffer, size_l);
				
				ptr_payload = ptr_payload + size_l;
			} while ((sizePreamble - size_prec) > 0);
	}

	
#if DEBUG > 3
	printf("TCPrecvPacket: First %d byte of TCP payload (preamble included): \n",sizePreamble);
	for(int y=0;y<sizePreamble;y++)
		printf("%.2X ",(int)payload[y]);
	printf("\n");
	fflush(stdout);
#endif
	

	

	
	if (payloadLogType != PL_NONE) { 						
		
		
		size = ntohl(*(uint32_t *) ptrToSizeInPayload); 
		PRINTD(1,"Expected TCP packet size %d \n",size);
		
		if (size > sizePreamble) {
			size_p = recv(newSock, (char *) buffer, (size - sizePreamble), 0);
		}
	} else {
		size_p = recv(newSock, (char *) buffer, MAX_PAYLOAD_SIZE, 0); 		
		size = size_p; 								
	}
	if (size_p <= 0) {
		return size_p;
	}
	PRINTD(2,"TCPrecvPacket: Received TCP Payload  :%d \n",size_p);

	
	if (size_p < (size - sizePreamble)) {
		
		int size_l = 0;
		
		int size_prec = size_p;
		
		while (size_prec < (size - sizePreamble)) {
			
			size_l = recv(newSock, (char *) buffer, ((size - sizePreamble) - size_prec), 0);

			PRINTD(2,"TCPrecvPacket: Received payload for other times: %d \n",size_l);

			if (size_l <= 0) {
				return size_l;
			}

			PRINTD(2,"TCPrecvPacket: Received Other data of TCP Payload\n");
			
			size_prec = size_l + size_prec;
		}
	}
	return size;
}


#ifdef SCTP

inline int SCTPrecvPacket(unsigned char *payload, int newSock, unsigned int stream, int preambleSize, int payloadLogType)
{
	
	

	PRINTD(2,"SCTPrecvPacket: Enter function SCTPrecvPacket \n");

	unsigned char *ptrToSizeInPayload = NULL;

	if (payloadLogType == PL_STANDARD)
		ptrToSizeInPayload = payload + 4 * sizeof(uint32_t);
	else if (payloadLogType == PL_SHORT)
		ptrToSizeInPayload = payload + 2 * sizeof(uint32_t);


   	
	int sizePreamble =  preambleSize;
	PRINTD(2,"SCTPrecvPacket: sizePreamble: %d\n", sizePreamble);

	
	unsigned char buffer[MAX_PAYLOAD_SIZE];

   	 

	

   	 
	int size = 0, size_p = 0, size_r = 0;

	

	
		
                size_r = recv(newSock, (char *) payload, sizePreamble, 0);
	


	PRINTD(2,"SCTPrecvPacket: Received SCTP Preamble first time :%d \n",size_r);
	if (size_r < 0)
		perror("SCTPRecvPacket function");

	
	if (size_r < 0)
		return 1;

	
	if (size_r < sizePreamble) {
			
			int size_l = 0;
			
			int size_prec = size_r;
			
			unsigned char *ptr_payload;
			
			ptr_payload = payload + size_prec;

			unsigned char *ptr;
			
			ptr = buffer;

			
			while ((sizePreamble - size_prec) > 0) {
				
				
					
                                        size_l= recv(newSock, (char *) payload, sizePreamble, 0);
				

				if (size_l < 0)
				    return 1;
				PRINTDS(2,"SCTPrecvPacket: Received Other data of SCTP Preamble %d\n",size_l);

				
				size_prec = size_l + size_prec;
				
				memcpy(ptr_payload, ptr, size_l);
				
				ptr = ptr + size_l;
				ptr_payload = ptr_payload + size_l;
			}
	}

	

	if (payloadLogType != PL_NONE) { 
		
		
		size = ntohl(*(uint32_t *) ptrToSizeInPayload); 

		PRINTD(1,"Expected TCP packet size %d \n",size);

		
		if (size > sizePreamble) {
			size_p = recv(newSock, (char *) buffer, (size - sizePreamble), 0);
		}
	} else {
		size_p = recv(newSock, (char *) buffer, MAX_PAYLOAD_SIZE, 0); 		
		size = size_p; 
	}
	PRINTD(2,"SCTP Pkt Receiver : Received SCTP Payload  :%d \n",size_p);

	if (size_p < 0)
		return 2;

	
	if (size_p < (size - sizePreamble)) {
			
			int size_l = 0;
			
			int size_prec = size_p;
			
			while (size_prec < (size - sizePreamble)) {
				
				
				
					
				        size_l=recv(newSock,(char *) buffer,(size-sizePreamble),0);
                                

				PRINTDS(2,"SCTPrecvPacket: Received payload for other times: %d \n",size_l);
				if (size_l < 0)
				    return 2;
					PRINTDS(2,"SCTPrecvPacket: Received Other data of SCTP Payload\n");
				
				size_prec = size_l + size_prec;
			}
	}
	
	PRINTD(2,"SCTPrecvPacket: Exit function SCTPrecvPacket \n");
	return size;
}
#endif

#ifdef DCCP

#endif


#ifdef WIN32
#define restrict
char *strtok_r (char *restrict string, const char *restrict delim, char **saveptr);
#endif



#define MAX_POLL 	1	


inline int ms_sleep(struct timeval wait)
{
#if defined WIN32
    fd_set dummy;

    SOCKET s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    FD_ZERO(&dummy);
    FD_SET(s, &dummy);
    select(0, 0, 0, &dummy, &wait);
    closesocket(s);
#elif defined UNIX
    select(0, NULL, NULL, NULL, &wait);
#endif
    PRINTD(2,"ms_sleep: WakeUP: %lu.%06lu\n", wait.tv_sec, (unsigned long int)wait.tv_usec);

    return 0;
}

void printVersion(const char* ProgramName);
