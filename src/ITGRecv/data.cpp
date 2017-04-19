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



#include "../common/ITG.h"
#include "../common/debug.h"
#include "../common/serial.h"
#include "../common/pipes.h"
#include "../common/thread.h"
#include "../common/timestamp.h"
#include "ITGRecv.h"
#include "data.h"

#ifdef UNIX
#include <netinet/ip.h>
#endif

#ifdef SCTP
extern int sctpSessionCount;
extern sctpSession sctpSessions[];
extern pthread_mutex_t mutexSctp;
#endif



void *icmpSock(void *param)
		{
	PRINTD(1,"icmpSock: icmpSock started\n");
#ifdef WIN32
	
	int first = 1;
#endif
	
	int dimheader = 0;

	
	HANDLE hComm = 0;

	
	paramThread *paraThread;
	
	paraThread = (paramThread *) param;
	
	int sock = 0;
	
	unsigned char buffer[MAX_PAYLOAD_SIZE + 32];
	
	unsigned char payload[MAX_PAYLOAD_SIZE];
	
	struct info *infos = (struct info *) calloc(logbuffer_size, sizeof(info));
	
	struct addrinfo SrcAddress;

	
	int size_r = 0;
	
	struct timeval RcvTime;
#ifdef WIN32
	
	struct addrinfo* sockAddress;

	

	
	LARGE_INTEGER _tstart, _tend;
	

	
	unsigned long secs = 0, msecs = 0;
#endif

	
	char *ptrSeqNum = (char *) payload + sizeof(uint32_t);                  
	
	char *ptrTimeSec = ptrSeqNum + sizeof(uint32_t);						
	
	char *ptrTimeUsec = ptrTimeSec + sizeof(uint32_t);						

	
	paraThread->addressInfos = infos;
	
	paraThread->count = 0;

	if (strcmp(paraThread->serial, "noSerial") != 0) {
		hComm = serialUp(paraThread->serial);
		if (hComm == INVALID_HANDLE_VALUE)
			printf("Error opening interface %s \n", paraThread->serial);
	}

	
	sock = socket(paraThread->destHost.ai_family, SOCK_RAW,
			(paraThread->destHost.ai_family == AF_INET) ? IPPROTO_ICMP : IPPROTO_ICMPV6);
	if (sock < 0) {
		
		struct pipeMsg msg;
		printf("icmpSock - error into create socket");

		msg.code = MSG_FT_ERR2;
		msg.flowId = paraThread->flowId;
		if (sendPipeMsg(paraThread->rPipe, &msg) < 0) {
			printf(" sending msg error");
		}
		sleep(INFINITE);
	}

	if (paraThread->dsByte
			&& (paraThread->destHost.ai_family == AF_INET)
			&& setsockopt(sock, SOL_IP, IP_TOS, (char *) &paraThread->dsByte, sizeof(BYTE)) < 0) {
		printf("** WARNING ** Flow %d. Could not set DS byte to: %d\n", paraThread->flowId, paraThread->dsByte);
	}

#ifdef WIN32
	
#ifdef IPv6RECV
	if ( getaddrinfo("::", NULL, &hint, &sockAddress) != 0)
	   reportErrorAndExit("icmpSock","getaddrinfo IPv6","Cannot set the default IP log address");
	
#else
	if ( getaddrinfo("0.0.0.0", NULL, &hint, &sockAddress) != 0)
	   reportErrorAndExit("icmpSock","getaddrinfo IPv4","Cannot set the default IP log address");
#endif
	
	if ( bind(sock, sockAddress->ai_addr, sockAddress->ai_addrlen))
	   reportErrorAndExit("icmpSock","bind","Cannot bind a socket on port");
#endif
#if defined UNIX && ! defined BSD
	
	if (paraThread->iface) {
		printf("Binding to device %s\n", paraThread->iface);
		if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, paraThread->iface, strlen(paraThread->iface)) < 0) {
			printf("** WARNING ** Cannot bind to device %s (hint: you must be root)\n", paraThread->iface);
			fflush(stdout);
		}
	}
#endif
	
	
	struct pipeMsg msg;
	msg.code = MSG_FT_OK;
	msg.flowId = paraThread->flowId;
	if (sendPipeMsg(paraThread->rPipe, &msg) < 0) {
		printf("error into sending msg to signal manager");
	}

	fprintf(stderr, "Listening to ICMP traffic\n");
	
	paraThread->socketClose = sock;
	
	SrcAddress.ai_family = paraThread->destHost.ai_family;
	
	if (SrcAddress.ai_family == PF_INET) {
		
		SrcAddress.ai_addr = (struct sockaddr *) malloc(sizeof(struct sockaddr_in));
		
		SrcAddress.ai_addrlen = sizeof(struct sockaddr_in);
		
		dimheader = sizeof(icmp) + sizeof(iphdr);

		
	} else if (SrcAddress.ai_family == PF_INET6) {
		
		SrcAddress.ai_addr = (struct sockaddr *) malloc(sizeof(struct sockaddr_in6));
		
		SrcAddress.ai_addrlen = sizeof(struct sockaddr_in6);
		
		dimheader = sizeof(icmpv6);
	}

	pthread_cleanup_push(free, SrcAddress.ai_addr);

		int firstpacket = 1;
		char HelpSrcAddress[INET6_ADDRSTRLEN];
		char HelpDstAddress[INET6_ADDRSTRLEN];
		int tmpPort_SrcPort = 0;
		int tmpPort_DstPort = 0;
		
		TSTART(_tstart, secs, msecs, first, 0, RECEIVER);

		PRINTD(1,"icmpSock: main loop\n");
		
		while (1) {
			
			size_r = recvfrom(sock, (char *) buffer, MAX_PAYLOAD_SIZE + dimheader, 0,
						SrcAddress.ai_addr, (socklen_t *) &SrcAddress.ai_addrlen);

			PRINTD(2,"icmpSock: Received RAW Packet :%d\n", size_r);

			if (size_r < 0)
				reportErrorAndExit("icmpSock", "recvfrom", "Cannot receive RAW packets");
			
			char *ptr = (char *) buffer;

			
			ptr = ptr + dimheader;

			
			size_r = size_r - dimheader;

			
			memcpy(&payload, ptr, size_r);

			if (hComm > 0) {
				DTR_Disable(hComm);
				DTR_Enable(hComm);
			}

			
			GET_TIME_OF_DAY(&RcvTime, _tend, _tstart, secs, msecs, 0, RECEIVER);

			

			
			if ((logCheck != 0) || (logRemote != 0)) {
				if (firstpacket == 1) {
					getInfo(&SrcAddress, tmpPort_SrcPort, HelpSrcAddress);
					getInfo(&paraThread->destHost, tmpPort_DstPort, HelpDstAddress);
					firstpacket = 0;
				}

				if (logCheck != 0) {
					int net_TimeSec = ntohl(*(uint32_t *) ptrTimeSec); 
					int net_TimeUsec = ntohl(*(uint32_t *) ptrTimeUsec); 

					if (paraThread->payloadLogType == PL_STANDARD) {	
						writeInBufferStandard(&infos[paraThread->count], *(uint32_t *) payload,
								*(uint32_t *) ptrSeqNum, HelpSrcAddress, HelpDstAddress,
								tmpPort_SrcPort, tmpPort_DstPort, net_TimeSec, RcvTime.tv_sec % 86400L,
								net_TimeUsec, RcvTime.tv_usec, size_r); 				
					} else if (paraThread->payloadLogType == PL_SHORT) {	
						writeInBufferShort(&infos[paraThread->count], *(uint32_t *) payload,
								*(uint32_t *) ptrSeqNum, HelpSrcAddress, HelpDstAddress,
								tmpPort_SrcPort, tmpPort_DstPort, RcvTime.tv_sec % 86400L,
								RcvTime.tv_usec, size_r);						
					} else {
						
						
						writeInBufferNone(&infos[paraThread->count], paraThread->flowId, HelpSrcAddress,
								HelpDstAddress, tmpPort_SrcPort, tmpPort_DstPort,
								RcvTime.tv_sec % 86400L, RcvTime.tv_usec, size_r);			
					}

				}
				
				
				
				if (logRemote != 0) {
					int net_TimeSec = ntohl(*(uint32_t *) ptrTimeSec); 	
					int net_TimeUsec = ntohl(*(uint32_t *) ptrTimeUsec); 	

					if (paraThread->payloadLogType == PL_STANDARD) {	
						writeInBufferStandard(&infos[paraThread->count], *(uint32_t *) payload,
								*(uint32_t *) ptrSeqNum, HelpSrcAddress, HelpDstAddress,
								tmpPort_SrcPort, tmpPort_DstPort, net_TimeSec, RcvTime.tv_sec % 86400L,
								net_TimeUsec, RcvTime.tv_usec, size_r); 				
					} else if (paraThread->payloadLogType == PL_SHORT) {	
						writeInBufferShort(&infos[paraThread->count], *(uint32_t *) payload,
								*(uint32_t *) ptrSeqNum, HelpSrcAddress, HelpDstAddress,
								tmpPort_SrcPort, tmpPort_DstPort, RcvTime.tv_sec % 86400L,
								RcvTime.tv_usec, size_r); 						
					} else {						
						
						writeInBufferNone(&infos[paraThread->count], paraThread->flowId, HelpSrcAddress,
								HelpDstAddress, tmpPort_SrcPort, tmpPort_DstPort,
								RcvTime.tv_sec % 86400L, RcvTime.tv_usec, size_r);
					}
					infosHostToNet(&infos[paraThread->count]);
				}

				
				paraThread->count++;
				
				if (paraThread->count == logbuffer_size) {
					
					if (logCheck != 0)
						flushBuffer((ofstream *) paraThread->fileLog, infos, paraThread->count);
					
					else
					if (logRemote != 0)
							{
						MUTEX_THREAD_LOCK(mutexLogRem);

						if (sendto(paraThread->logSock, (char *) infos,
								paraThread->count * sizeof(struct info), 0,
								paraThread->logHost->ai_addr, paraThread->logHost->ai_addrlen) < 0)
							reportErrorAndExit("icmpSock", "sendto", "Cannot send log infos to LogServer");

						paraThread->count = 0;

						MUTEX_THREAD_UNLOCK(mutexLogRem);

						PRINTD(1,"icmpSock: Sent Infos to LogServer\n");
					}
				} 
			} 
		} 

		pthread_cleanup_pop(1);
	return NULL;
} 


void *udpSock(void *param)
		{
	PRINTD(1,"udpSock: udpSock started\n");
#ifdef WIN32
	
	int first = 1;
#endif
	
	HANDLE hComm = 0;

	
	paramThread *paraThread;

	
	paraThread = (paramThread *) param;
	
	int sock = 0;
	
	unsigned char payload[MAX_PAYLOAD_SIZE];
	
	struct info *infos = (struct info *) calloc(logbuffer_size, sizeof(info));
	
	struct addrinfo SrcAddress;
	
	in_port_t tmpPort = 0;
	
	int size_r = 0;
	
	unsigned char *ptrSeqNum = payload + sizeof(uint32_t);			
	
	unsigned char *ptrTimeSec = ptrSeqNum + sizeof(uint32_t);		
	
	unsigned char *ptrTimeUsec = ptrTimeSec + sizeof(uint32_t);		
	
	struct timeval RcvTime;
#ifdef WIN32
	

	
	LARGE_INTEGER _tstart, _tend;
	

	
	unsigned long secs = 0, msecs = 0;
#endif
	
	paraThread->addressInfos = infos;
	
	paraThread->count = 0;

	if (strcmp(paraThread->serial, "noSerial") != 0) {
		hComm = serialUp(paraThread->serial);
		if (hComm == INVALID_HANDLE_VALUE)
			printf("Error opening interface %s \n", paraThread->serial);
	}

	bool socketAlreadyOpen = false;
#ifdef MULTIPORT

	if ((passiveMode == false) && (paraThread->indexPort > 0))
			{
		MUTEX_THREAD_LOCK(sharedUdpSockets[paraThread->indexPort].mutexSharedSockets);

		sock = sharedUdpSockets[paraThread->indexPort].socket;

		if (sock > 0)
				{
			socketAlreadyOpen = true;
		}
	}

#endif

	
	if (socketAlreadyOpen == false) {
		sock = socket(paraThread->destHost.ai_family, SOCK_DGRAM, 0);
		if (paraThread->dsByte
				&& (paraThread->destHost.ai_family == AF_INET)
				&& setsockopt(sock, SOL_IP, IP_TOS, (char *) &paraThread->dsByte, sizeof(BYTE)) < 0) {
			printf("** WARNING ** Flow %d. Could not set DS byte to: %d\n", paraThread->flowId, paraThread->dsByte);
		}
	}

	if (sock < 0)
		reportErrorAndExit("udpSock", "socket", "Cannot create a DATAGRAM socket on port");

	if ((passiveMode == false) && (socketAlreadyOpen == false))
			{

		if (bind(sock, paraThread->destHost.ai_addr, paraThread->destHost.ai_addrlen) < 0) {
			
			printf("Error into bind function!\n");
			struct pipeMsg msg;
			msg.code = MSG_FT_ERR1;
			msg.flowId = paraThread->flowId;
			if (sendPipeMsg(paraThread->rPipe, &msg) < 0) {
				printf(" sending msg error");
			}
			sleep(INFINITE);
		}
		else
		{
			
#ifdef WIN32
			int len=paraThread->destHost.ai_addrlen;
			getsockname(sock,paraThread->destHost.ai_addr,&len);
			paraThread->destHost.ai_addrlen=len;
#else
			getsockname(sock, paraThread->destHost.ai_addr, &(paraThread->destHost.ai_addrlen));
#endif

			
			GET_PORT((&(paraThread->destHost)), tmpPort);
			fprintf(stderr, "Listening on UDP port : %d\n", ntohs(tmpPort));
			fflush(stderr);

#ifdef MULTIPORT
			if (paraThread->indexPort == 0)
					{
				paraThread->indexPort = ntohs(tmpPort);
				MUTEX_THREAD_LOCK(sharedUdpSockets[paraThread->indexPort].mutexSharedSockets);
			}
#endif

			
			struct pipeMsg msg;
			msg.code = MSG_FT_OK;
			msg.flowId = paraThread->flowId;
			if (sendPipeMsg(paraThread->rPipe, &msg) < 0) {
				printf("error sending msg to signal manager");
			}
		}

	}
	
	else if (passiveMode == true) {
		struct addrinfo *addrForListen = NULL;

#if (defined WIN32 && defined IPv6RECV) || defined UNIX || defined BSD
		if (paraThread->destHost.ai_family == PF_INET6) {
			getaddrinfo("::", NULL, &hint, &addrForListen);
		} else
#endif
		{
			getaddrinfo("0.0.0.0", NULL, &hint, &addrForListen);
		}

		
		SET_PORT((addrForListen), htons((paraThread->portForPssv)));

		
		if (bind(sock, addrForListen->ai_addr, addrForListen->ai_addrlen) < 0) {
			
			struct pipeMsg msg;

			msg.code = MSG_FT_ERR1;
			msg.flowId = paraThread->flowId;
			if (sendPipeMsg(paraThread->rPipe, &msg) < 0) {
				printf(" sending msg error");
			}
			sleep(INFINITE); 
		} else {
			
			GET_PORT(( addrForListen), tmpPort);
			printf("Listening on UDP port : %d\n", ntohs(tmpPort));
			fflush(stdout);
			
			for (int x = 0; x < numHolePkt; x++) {
				if (sendto(sock, "hello", sizeof("hello"), 0, paraThread->destHost.ai_addr,
						paraThread->destHost.ai_addrlen) < 0) {
					reportErrorAndExit("udpSock", "sendto", "Cannot sendto (Passive Mode --> Hole punching)");
				}
			}
		}

		
		if (connect(sock, paraThread->destHost.ai_addr, paraThread->destHost.ai_addrlen) < 0) {
			reportErrorAndExit("udpSock", "connect", "Cannot connect (Passive Mode)");
		}
		freeaddrinfo(addrForListen);
	}
#ifdef MULTIPORT
	else if ((passiveMode == false) && (socketAlreadyOpen == true)) {
		
		struct pipeMsg msg;
		msg.code = MSG_FT_OK;
		msg.flowId = paraThread->flowId;
		if (sendPipeMsg(paraThread->rPipe, &msg) < 0) {
			printf("error sending msg to signal manager");
		}
	}

	if (passiveMode == false) {
		
		if (socketAlreadyOpen == false) {
			sharedUdpSockets[paraThread->indexPort].socket = sock;
		}

		sharedUdpSockets[paraThread->indexPort].inUse++;

		MUTEX_THREAD_UNLOCK(sharedUdpSockets[paraThread->indexPort].mutexSharedSockets);
	}
#endif
	

#if defined UNIX && ! defined BSD
	
	if ((paraThread->iface) && (socketAlreadyOpen == false)) {
		printf("Binding to device %s\n", paraThread->iface);
		if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, paraThread->iface, strlen(paraThread->iface)) < 0) {
			printf("** WARNING ** Cannot bind to device %s (hint: you must be root)\n", paraThread->iface);
			fflush(stdout);
		}
	}
#endif

	
	paraThread->socketClose = sock;
	
	SrcAddress.ai_family = paraThread->destHost.ai_family;
	
	if (SrcAddress.ai_family == PF_INET) {
		
		SrcAddress.ai_addr = (struct sockaddr *) malloc(sizeof(struct sockaddr_in));
		
		SrcAddress.ai_addrlen = sizeof(struct sockaddr_in);
		
	} else if (SrcAddress.ai_family == PF_INET6) {
		
		SrcAddress.ai_addr = (struct sockaddr *) malloc(sizeof(struct sockaddr_in6));
		
		SrcAddress.ai_addrlen = sizeof(struct sockaddr_in6);
	}

	pthread_cleanup_push(free, SrcAddress.ai_addr);

		int firstpacket = 1;
		char HelpSrcAddress[INET6_ADDRSTRLEN];
		char HelpDstAddress[INET6_ADDRSTRLEN];
		int tmpPort_SrcPort = 0;
		int tmpPort_DstPort = 0;

		
		
		TSTART(_tstart, secs, msecs, first, 0, RECEIVER);

		PRINTD(1,"udpSock: main loop\n");
		
		while (1) {
			if (passiveMode == false) {									
				
				size_r = recvfrom(sock, (char *) payload, MAX_PAYLOAD_SIZE, 0, SrcAddress.ai_addr,
						(socklen_t *) &SrcAddress.ai_addrlen);
			} else {
						
				size_r = recvfrom(sock, (char *) payload, MAX_PAYLOAD_SIZE, 0, NULL, NULL);	
			}												
			PRINTD(2,"udpSock: Received DATAGRAM packet\n");

			if (size_r < 0) {
				PRINTD(1,"\nudpSock: Error:%s\n",strerror(errno));
				
				
#ifndef OSX
				reportErrorAndExit("udpSock", "recvfrom", "Cannot receive UDP packets");
#else
				sleep(INFINITE);
#endif
			}

			if (hComm > 0) {
				DTR_Disable(hComm);
				DTR_Enable(hComm);
			}

			
			GET_TIME_OF_DAY(&RcvTime, _tend, _tstart, secs, msecs, 0, RECEIVER);
			

			
			if ((logCheck != 0) || (logRemote != 0)) {
				if (firstpacket == 1) {
					if (passiveMode == false) {
						getInfo(&SrcAddress, tmpPort_SrcPort, HelpSrcAddress);
						getInfo(&paraThread->destHost, tmpPort_DstPort, HelpDstAddress);
					} else {
						
#ifdef WIN32
						int len=SrcAddress.ai_addrlen;						
						getsockname(sock,SrcAddress.ai_addr,&len);
						SrcAddress.ai_addrlen=len;
#else
						getsockname(sock, SrcAddress.ai_addr, &SrcAddress.ai_addrlen);		
#endif
						getInfo(&paraThread->destHost, tmpPort_SrcPort, HelpSrcAddress);	
						getInfo(&SrcAddress, tmpPort_DstPort, HelpDstAddress);			
					}
					firstpacket = 0;
				}
#ifdef MULTIPORT
				else if (passiveMode == false) {
					getInfo(&SrcAddress, tmpPort_SrcPort, HelpSrcAddress);
				}
#endif
				
				if (logCheck != 0) {
					int net_TimeSec = ntohl(*(uint32_t *) ptrTimeSec); 
					int net_TimeUsec = ntohl(*(uint32_t *) ptrTimeUsec); 

					if (paraThread->payloadLogType == PL_STANDARD) { 	
						writeInBufferStandard(&infos[paraThread->count], *(uint32_t *) payload,
								*(uint32_t *) ptrSeqNum,
								HelpSrcAddress, HelpDstAddress, tmpPort_SrcPort, tmpPort_DstPort,
								net_TimeSec,
								RcvTime.tv_sec % 86400L, net_TimeUsec, RcvTime.tv_usec, size_r);	
					} else if (paraThread->payloadLogType == PL_SHORT) {	
						writeInBufferShort(&infos[paraThread->count], *(uint32_t *) payload,
								*(uint32_t *) ptrSeqNum,
								HelpSrcAddress, HelpDstAddress, tmpPort_SrcPort, tmpPort_DstPort,
								RcvTime.tv_sec % 86400L,
								RcvTime.tv_usec, size_r);						
					} else {
						
						
						writeInBufferNone(&infos[paraThread->count], paraThread->flowId, HelpSrcAddress,
								HelpDstAddress,
								tmpPort_SrcPort, tmpPort_DstPort, RcvTime.tv_sec % 86400L,
								RcvTime.tv_usec, size_r);
					}
				}
				
				
				
				if (logRemote != 0) {
					int net_TimeSec = ntohl(*(uint32_t *) ptrTimeSec); 
					int net_TimeUsec = ntohl(*(uint32_t *) ptrTimeUsec); 

					if (paraThread->payloadLogType == PL_STANDARD) { 	
						writeInBufferStandard(&infos[paraThread->count], *(uint32_t *) payload,
								*(uint32_t *) ptrSeqNum, HelpSrcAddress, HelpDstAddress,
								tmpPort_SrcPort, tmpPort_DstPort, net_TimeSec, RcvTime.tv_sec % 86400L,
								net_TimeUsec, RcvTime.tv_usec, size_r); 				
					} else if (paraThread->payloadLogType == PL_SHORT) {	
						writeInBufferShort(&infos[paraThread->count], *(uint32_t *) payload,
								*(uint32_t *) ptrSeqNum, HelpSrcAddress, HelpDstAddress,
								tmpPort_SrcPort, tmpPort_DstPort, RcvTime.tv_sec % 86400L,
								RcvTime.tv_usec, size_r); 						
					} else {
						
						
						writeInBufferNone(&infos[paraThread->count], paraThread->flowId, HelpSrcAddress,
								HelpDstAddress, tmpPort_SrcPort, tmpPort_DstPort,
								RcvTime.tv_sec % 86400L, RcvTime.tv_usec, size_r);
					}
					infosHostToNet(&infos[paraThread->count]);
				}
				
				
				paraThread->count++;
				
				if (paraThread->count == logbuffer_size) {
					
					if (logCheck != 0)
						flushBuffer((ofstream *) paraThread->fileLog, infos, paraThread->count);
					
					else
					if (logRemote != 0)
							{
						MUTEX_THREAD_LOCK(mutexLogRem);

						if (sendto(paraThread->logSock, (char *) infos,
								paraThread->count * sizeof(struct info), 0,
								paraThread->logHost->ai_addr, paraThread->logHost->ai_addrlen) < 0)
							reportErrorAndExit("udpSock", "sendto", "Cannot send log infos to LogServer");

						paraThread->count = 0;

						MUTEX_THREAD_UNLOCK(mutexLogRem);

						PRINTD(1,"udpSock: Sent Infos to LogServer\n");
					}
				} 
			} 
			
			if (paraThread->meter == METER_RTTM) {
				if (passiveMode == false) {								
					if (sendto(sock, (char *) payload, size_r, 0, SrcAddress.ai_addr, SrcAddress.ai_addrlen) < 0)
						reportErrorAndExit("udpSock", "sendto", "Cannot send back payload for rttm");
				} else {										
					if (sendto(sock, (char *) payload, size_r, 0, NULL, 0) < 0)			
						reportErrorAndExit("udpSock", "sendto",
								"Cannot send back payload for rttm (Passive Mode)");	
				}											
				PRINTD(2,"udpSock: Sent RTTM message\n");
			} 
		} 
		pthread_cleanup_pop(1);
	return NULL;
} 


void *tcpSock(void *param)
		{
	PRINTD(1,"tcpSock: tcpSock started\n");
#ifdef WIN32
	
	int first = 1;
#endif
	
	HANDLE hComm = 0;

	
	paramThread *paraThread;
	
	paraThread = (paramThread *) param;

	
	int sock = 0;

	
	unsigned char payload[MAX_PAYLOAD_SIZE];
	
	struct info *infos = (struct info *) calloc(logbuffer_size, sizeof(info));
	
	struct addrinfo SrcAddress;
	
	in_port_t tmpPort = 0;
	
	int newSock = 0;
	
	int size = 0;
	
	unsigned char *ptrSeqNum = payload + sizeof(uint32_t);			
	
	unsigned char *ptrTimeSec = ptrSeqNum + sizeof(uint32_t);		
	
	unsigned char *ptrTimeUsec = ptrTimeSec + sizeof(uint32_t);		
	
	struct timeval RcvTime;
#ifdef WIN32
	

	
	LARGE_INTEGER _tstart, _tend;
	

	
	unsigned long secs = 0, msecs = 0;
#endif
	
	paraThread->addressInfos = infos;
	
	paraThread->count = 0;

	
	memset(payload, 0, MAX_PAYLOAD_SIZE);

	if (strcmp(paraThread->serial, "noSerial") != 0) {
		hComm = serialUp(paraThread->serial);
		if (hComm == INVALID_HANDLE_VALUE)
			printf("Error opening interface %s \n", paraThread->serial);
	}

	bool socketAlreadyOpen = false;
#ifdef MULTIPORT

	if ((passiveMode == false) && (paraThread->indexPort > 0)) {
		MUTEX_THREAD_LOCK(sharedTcpSockets[paraThread->indexPort].mutexSharedSockets);

		sock = sharedTcpSockets[paraThread->indexPort].socket;

		if (sock > 0) {
			socketAlreadyOpen = true;
		}
	}

#endif

	
	if (socketAlreadyOpen == false) {
		sock = socket(paraThread->destHost.ai_family, SOCK_STREAM, 0);
		if (paraThread->dsByte
				&& (paraThread->destHost.ai_family == AF_INET)
				&& setsockopt(sock, SOL_IP, IP_TOS, (char *) &paraThread->dsByte, sizeof(BYTE)) < 0) {
			printf("** WARNING ** Flow %d. Could not set DS byte to: %d\n", paraThread->flowId, paraThread->dsByte);
		}
	}

	if (sock < 0)
		reportErrorAndExit("tcpSock", "socket", "Cannot create a STREAM socket on port");

	if ((passiveMode == false) && (socketAlreadyOpen == false)) { 	

		
		if (bind(sock, paraThread->destHost.ai_addr, paraThread->destHost.ai_addrlen) < 0) {
			
			struct pipeMsg msg;

			msg.code = MSG_FT_ERR1;
			msg.flowId = paraThread->flowId;
			if (sendPipeMsg(paraThread->rPipe, &msg) < 0) {
				printf(" sending msg error");
			}
			sleep(INFINITE); 
		}

	}

#if defined UNIX && ! defined BSD
	
	if ((paraThread->iface) && (socketAlreadyOpen == false)) {
		printf("Binding to device %s\n", paraThread->iface);
		if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, paraThread->iface, strlen(paraThread->iface)) < 0) {
			printf("** WARNING ** Cannot bind to device %s (hint: you must be root)\n", paraThread->iface);
			fflush(stdout);
		}
	}
#endif

	if ((passiveMode == false) && (socketAlreadyOpen == false)) {

		
		if (listen(sock, SOMAXCONN) < 0)
			reportErrorAndExit("tcpSock", "listen", "Cannot listen on a port");
		else {
			
#ifdef WIN32
			int len=paraThread->destHost.ai_addrlen;
			getsockname(sock,paraThread->destHost.ai_addr,&len);
			paraThread->destHost.ai_addrlen=len;
#else
			getsockname(sock, paraThread->destHost.ai_addr, &(paraThread->destHost.ai_addrlen));
#endif
			
			GET_PORT((&(paraThread->destHost)), tmpPort);
			printf("Listening on TCP port : %d\n", ntohs(tmpPort));
			fflush(stdout);

#ifdef MULTIPORT
			if (paraThread->indexPort == 0)
					{
				paraThread->indexPort = ntohs(tmpPort);
				MUTEX_THREAD_LOCK(sharedTcpSockets[paraThread->indexPort].mutexSharedSockets);
			}
#endif

			
			struct pipeMsg msg;
			msg.code = MSG_FT_OK;
			msg.flowId = paraThread->flowId;
			if (sendPipeMsg(paraThread->rPipe, &msg) < 0) {
				printf("error into sending msg to signal manager");
			}
		}

	}
	#ifdef MULTIPORT
	else if ((passiveMode == false) && (socketAlreadyOpen == true))
			{
		
		struct pipeMsg msg;
		msg.code = MSG_FT_OK;
		msg.flowId = paraThread->flowId;
		if (sendPipeMsg(paraThread->rPipe, &msg) < 0) {
			printf("error into sending msg to signal manager");
		}
	}
#endif

	
	SrcAddress.ai_family = paraThread->destHost.ai_family;
	
	if (SrcAddress.ai_family == PF_INET) {
		
		SrcAddress.ai_addr = (struct sockaddr *) malloc(sizeof(struct sockaddr_in));
		
		SrcAddress.ai_addrlen = sizeof(struct sockaddr_in);
		
	} else if (SrcAddress.ai_family == PF_INET6) {
		
		SrcAddress.ai_addr = (struct sockaddr *) malloc(sizeof(struct sockaddr_in6));
		
		SrcAddress.ai_addrlen = sizeof(struct sockaddr_in6);
	}

	pthread_cleanup_push(free, SrcAddress.ai_addr);

		if (passiveMode == false) { 	

			

			if ((newSock = accept(sock, SrcAddress.ai_addr, (socklen_t *) &SrcAddress.ai_addrlen)) < 0)
				reportErrorAndExit("tcpSock", "accept", "Cannot accept connection");

#ifdef MULTIPORT
			
			if ((passiveMode == false) && (socketAlreadyOpen == false))
					{
				sharedTcpSockets[paraThread->indexPort].socket = sock;
			}

			sharedTcpSockets[paraThread->indexPort].inUse++;

			MUTEX_THREAD_UNLOCK(sharedTcpSockets[paraThread->indexPort].mutexSharedSockets);
#else
			
			if ( closeSock(sock) == -1)
			reportErrorAndExit("tcpSock","closeSock","Cannot close socket sock");
#endif

			
			paraThread->socketClose = newSock;

			
		} else {
			struct addrinfo *SrcAddrForConnect = NULL;

#if (defined WIN32 && defined IPv6RECV) || defined UNIX || defined BSD
			if (paraThread->destHost.ai_family == PF_INET6) {
				getaddrinfo("::", NULL, &hint, &SrcAddrForConnect);
			} else
#endif
			{
				getaddrinfo("0.0.0.0", NULL, &hint, &SrcAddrForConnect);
			}

			
			SET_PORT((SrcAddrForConnect), htons((paraThread->portForPssv)));

			
			if (bind(sock, SrcAddrForConnect->ai_addr, SrcAddrForConnect->ai_addrlen) < 0) {
				
				struct pipeMsg msg;

				msg.code = MSG_FT_ERR1;
				msg.flowId = paraThread->flowId;
				if (sendPipeMsg(paraThread->rPipe, &msg) < 0) {
					printf(" sending msg error");
				}
				sleep(INFINITE); 
			}

			
			if (connect(sock, paraThread->destHost.ai_addr, paraThread->destHost.ai_addrlen) < 0)
				reportErrorAndExit("tcpSock", "connect", "Cannot connect (Passive Mode)");
			PRINTD(1,"tcpSock: Connection establishes (Passive Mode)\n");

			freeaddrinfo(SrcAddrForConnect);

			
			newSock = sock;

			
			paraThread->socketClose = sock;

			
#ifdef WIN32
			int len=SrcAddress.ai_addrlen;						
			getsockname(sock,SrcAddress.ai_addr,&len);				
			SrcAddress.ai_addrlen=len;						
#else
			getsockname(sock, SrcAddress.ai_addr, &SrcAddress.ai_addrlen);		
#endif
		}
		

		int firstpacket = 1;
		char HelpSrcAddress[INET6_ADDRSTRLEN];
		char HelpDstAddress[INET6_ADDRSTRLEN];
		int tmpPort_SrcPort = 0;
		int tmpPort_DstPort = 0;

		
		TSTART(_tstart, secs, msecs, first, 0, RECEIVER);
		PRINTD(1,"tcpSock: main loop\n");
		
		while (1) {
			
			PRINTD(2, "tcpSock: preambleSize = %d \n",paraThread->preambleSize);
			size = TCPrecvPacket((unsigned char*) payload, newSock, paraThread->preambleSize, paraThread->payloadLogType);

			
			if (size <= 0) {
				PRINTD(1,"tcpSock: TCPrecvPacket() = %d\n",size);
				
				if (size < 0) {
					
					struct pipeMsg msg;

					if (passiveMode == false) {
						GET_PORT((&(paraThread->destHost)), tmpPort_DstPort);
					} else {
						GET_PORT((&SrcAddress), tmpPort_DstPort);
					}
					printf("Error on TCP port : %d\n", ntohs(tmpPort_DstPort));
					printf("Finish on TCP port : %d\n\n", ntohs(tmpPort_DstPort));
					fflush(stdout);

					msg.code = MSG_FT_ERR_SOCK;
					msg.flowId = paraThread->flowId;
					if (sendPipeMsg(paraThread->rPipe, &msg) < 0) {
						printf(" sending msg error");
					}
				}
				sleep(INFINITE);
			}

			if (hComm > 0) {
				DTR_Disable(hComm);
				DTR_Enable(hComm);
			}

			
			GET_TIME_OF_DAY(&RcvTime, _tend, _tstart, secs, msecs, 0, RECEIVER);

			
			
			if ((logCheck != 0) || (logRemote != 0)) {

				if (firstpacket == 1) {
					if (passiveMode == false) {							
						getInfo(&SrcAddress, tmpPort_SrcPort, HelpSrcAddress);
						getInfo(&paraThread->destHost, tmpPort_DstPort, HelpDstAddress);
					} else {									
						
#ifdef WIN32
						int len=SrcAddress.ai_addrlen;						
						getsockname(sock,SrcAddress.ai_addr,&len);
						SrcAddress.ai_addrlen=len;
#else
						getsockname(sock, SrcAddress.ai_addr, &SrcAddress.ai_addrlen);		
#endif
						getInfo(&paraThread->destHost, tmpPort_SrcPort, HelpSrcAddress);	
						getInfo(&SrcAddress, tmpPort_DstPort, HelpDstAddress);			
					}										
					firstpacket = 0;
				}
				
				if (paraThread->l7Proto == L7_PROTO_TELNET)
					size = size - 20; 

				if (logCheck != 0) {
					int net_TimeSec = ntohl(*(uint32_t *) ptrTimeSec); 
					int net_TimeUsec = ntohl(*(uint32_t *) ptrTimeUsec); 

					if (paraThread->payloadLogType == PL_STANDARD) {	
						writeInBufferStandard(&infos[paraThread->count], *(uint32_t *) payload,
								*(unsigned int *) ptrSeqNum, HelpSrcAddress, HelpDstAddress,
								tmpPort_SrcPort, tmpPort_DstPort, net_TimeSec, RcvTime.tv_sec % 86400L,
								net_TimeUsec, RcvTime.tv_usec, size);					
					} else if (paraThread->payloadLogType == PL_SHORT) {	
						writeInBufferShort(&infos[paraThread->count], *(uint32_t *) payload,
								*(unsigned int *) ptrSeqNum, HelpSrcAddress, HelpDstAddress,
								tmpPort_SrcPort, tmpPort_DstPort, RcvTime.tv_sec % 86400L,
								RcvTime.tv_usec, size);							
					} else {
						
						
						writeInBufferNone(&infos[paraThread->count], paraThread->flowId, HelpSrcAddress,
								HelpDstAddress, tmpPort_SrcPort, tmpPort_DstPort,
								RcvTime.tv_sec % 86400L, RcvTime.tv_usec, size);
					}
				}
				
				
				
				if (logRemote != 0) {
					int net_TimeSec = ntohl(*(uint32_t *) ptrTimeSec); 
					int net_TimeUsec = ntohl(*(uint32_t *) ptrTimeUsec); 

					if (paraThread->payloadLogType == PL_STANDARD) {	
						writeInBufferStandard(&infos[paraThread->count], *(uint32_t *) payload,
								*(uint32_t *) ptrSeqNum, HelpSrcAddress, HelpDstAddress,
								tmpPort_SrcPort, tmpPort_DstPort, net_TimeSec, RcvTime.tv_sec % 86400L,
								net_TimeUsec, RcvTime.tv_usec, size); 					
					} else if (paraThread->payloadLogType == PL_SHORT) { 	
						writeInBufferShort(&infos[paraThread->count], *(uint32_t *) payload,
								*(uint32_t *) ptrSeqNum, HelpSrcAddress, HelpDstAddress,
								tmpPort_SrcPort, tmpPort_DstPort, RcvTime.tv_sec % 86400L,
								RcvTime.tv_usec, size); 						
					} else {
						
						
						writeInBufferNone(&infos[paraThread->count], paraThread->flowId, HelpSrcAddress,
								HelpDstAddress, tmpPort_SrcPort, tmpPort_DstPort,
								RcvTime.tv_sec % 86400L, RcvTime.tv_usec, size);
					}
					infosHostToNet(&infos[paraThread->count]);
				}

				
				if (size != 0) paraThread->count++; 

				
				if (paraThread->count == logbuffer_size) {
					
					if (logCheck != 0)
						flushBuffer((ofstream *) paraThread->fileLog, infos, paraThread->count);
					
					else
					if (logRemote != 0)
							{
						MUTEX_THREAD_LOCK(mutexLogRem);

						if (sendto(paraThread->logSock, (char *) infos,
								paraThread->count * sizeof(struct info), 0,
								paraThread->logHost->ai_addr, paraThread->logHost->ai_addrlen) < 0)
							reportErrorAndExit("tcpSock", "sendto", "Cannot send log infos to LogServer");

						paraThread->count = 0;

						MUTEX_THREAD_UNLOCK(mutexLogRem);
						PRINTD(1,"tcpSock: Sent infos to LogServer\n");
					}
				} 
			} 
			
			if (paraThread->meter == METER_RTTM) {
				if (sendto(newSock, (char *) payload, size, 0, SrcAddress.ai_addr, SrcAddress.ai_addrlen) < 0)
					reportErrorAndExit("tcpSock", "sendto", "Cannot send payload back for rttm");
				PRINTD(2,"tcpSock: Sent RTTM infos\n");
			} 
		} 
		pthread_cleanup_pop(1);
	return NULL;
} 

#ifdef SCTP
void *sctpSock(void *param)
		{
	PRINTD(1,"sctpSock: sctpSock started\n");
#ifdef WIN32
	
	int first = 1;
#endif
	
	HANDLE hComm = 0;

	
	paramThread *paraThread;
	
	paraThread = (paramThread *) param;

	
	int sock = 0;

	
	unsigned char payload[MAX_PAYLOAD_SIZE];
	
	struct info *infos = (struct info *) calloc(logbuffer_size, sizeof(info));
	
	struct addrinfo SrcAddress;
	
	in_port_t tmpPort = 0;
	
	int newSock = 0;
	
	int size = 0;
	
	unsigned char *ptrSeqNum = payload + sizeof(uint32_t);			
	
	unsigned char *ptrTimeSec = ptrSeqNum + sizeof(uint32_t);		
	
	unsigned char *ptrTimeUsec = ptrTimeSec + sizeof(uint32_t);		
	
	struct timeval RcvTime;
#ifdef WIN32
	

	
	LARGE_INTEGER _tstart, _tend;
	

	
	unsigned long secs = 0, msecs = 0;
#endif


	int sctpId;
	bool newSession = true;


	PRINTD(1,"sctpSock: Start of Function. logCheck = %d logRemote = %d \n", logCheck, logRemote);

	
	paraThread->addressInfos = infos;
	
	paraThread->count = 0;

	
	memset(payload, 0, MAX_PAYLOAD_SIZE);

	if (strcmp(paraThread->serial, "noSerial") != 0) {
		hComm = serialUp(paraThread->serial);
		if (hComm == INVALID_HANDLE_VALUE)
			printf("Error opening interface %s \n", paraThread->serial);
	}


	unsigned int port = 0;
	GET_PORT((&(paraThread->destHost)), port);

	MUTEX_THREAD_LOCK(mutexSctp);

	
	for (sctpId = 0; sctpId < sctpSessionCount; sctpId++) {
		if (sctpSessions[sctpId].parsedStreams > 0 && sctpSessions[sctpId].port == port) {
			newSession = false;
			break;
		}
	}

	
	if (newSession) {
		PRINTD(1,"sctpSock: Receiving new SCTP session on port %d...\n", ntohs(port));
		sctpId = sctpSessionCount++;
		sctpSessions[sctpId].port = port;
		sctpSessions[sctpId].busyStreams = 0;
		sctpSessions[sctpId].parsedStreams = 1;
		sctpSessions[sctpId].sock = -1;
	}
	else
		sctpSessions[sctpId].parsedStreams++;

	

	PRINTD(1,"sctpSock: Receiving new SCTP stream...\n");

	

	if (newSession) {
		
		sock = socket(paraThread->destHost.ai_family, SOCK_STREAM, IPPROTO_SCTP);
		if (sock < 0) {
			reportErrorAndExit("sctpSock", "socket", "Cannot create a STREAM socket on port");
		}

		
		if (bind(sock, paraThread->destHost.ai_addr, paraThread->destHost.ai_addrlen) < 0) {
			
			struct pipeMsg msg;

			msg.code = MSG_FT_ERR1;
			msg.flowId = paraThread->flowId;
			if (sendPipeMsg(paraThread->rPipe, &msg) < 0) {
				printf(" sending msg error");
			}
			sleep(INFINITE);
		}

		
		if (listen(sock, SOMAXCONN) < 0) {
			reportErrorAndExit("tcpSock", "listen", "Cannot listen on a port");
		} else {
			
#ifdef WIN32
			int len = paraThread->destHost.ai_addrlen;
			getsockname(sock,paraThread->destHost.ai_addr, &len);
			paraThread->destHost.ai_addrlen = len;
#else
			getsockname(sock, paraThread->destHost.ai_addr, &(paraThread->destHost.ai_addrlen));
#endif

			
			GET_PORT((&(paraThread->destHost)), tmpPort);
			fprintf(stderr, "Listening on STCP port : %d\n", ntohs(tmpPort));
			fflush(stderr);

			
			struct pipeMsg msg;
			msg.code = MSG_FT_OK;
			msg.flowId = paraThread->flowId;
			if (sendPipeMsg(paraThread->rPipe, &msg) < 0) {
				printf("error into sending msg to signal manager");
			}
		}

		
		SrcAddress.ai_family = paraThread->destHost.ai_family;
		
		if (SrcAddress.ai_family == PF_INET) {
			
			SrcAddress.ai_addr = (struct sockaddr *) malloc(sizeof(struct sockaddr_in));
			
			SrcAddress.ai_addrlen = sizeof(struct sockaddr_in);
			
		} else if (SrcAddress.ai_family == PF_INET6) {
			
			SrcAddress.ai_addr = (struct sockaddr *) malloc(sizeof(struct sockaddr_in6));
			
			SrcAddress.ai_addrlen = sizeof(struct sockaddr_in6);
		}

		

		

		if ((newSock = accept(sock, SrcAddress.ai_addr, (socklen_t *) &SrcAddress.ai_addrlen)) < 0)
			reportErrorAndExit("sctpSock", "accept", "Cannot accept connection");
		
		if (closeSock(sock) == -1)
			reportErrorAndExit("sctpSock", "closeSock", "Cannot close socket sock");

		
		paraThread->socketClose = newSock;

		
		sctpSessions[sctpId].sock = newSock;
		
	} else {
		newSock = sctpSessions[sctpId].sock;
	}
	MUTEX_THREAD_UNLOCK(mutexSctp);

	int firstpacket = 1;
	char HelpSrcAddress[INET6_ADDRSTRLEN];
	char HelpDstAddress[INET6_ADDRSTRLEN];
	int tmpPort_SrcPort = 0;
	int tmpPort_DstPort = 0;

	
	TSTART(_tstart, secs, msecs, first, 0, RECEIVER);
	PRINTD(1,"sctpSock: main loop\n");
	
	while (1) {
		
		size = SCTPrecvPacket((unsigned char*) payload, newSock, sctpId, paraThread->preambleSize, paraThread->payloadLogType);
		PRINTD(2,"sctpSock: Received SCTP data. Size of the received data chinck: %d\n",size);

		if (hComm > 0) {
			DTR_Disable(hComm);
			DTR_Enable(hComm);
		}

		
		GET_TIME_OF_DAY(&RcvTime, _tend, _tstart, secs, msecs, 0, RECEIVER);

		
		
		if ((logCheck != 0) || (logRemote != 0)) {  
			
			PRINTD(2,"sctpSock: Log received data \n");
			if (firstpacket == 1) {
				getInfo(&SrcAddress, tmpPort_SrcPort, HelpSrcAddress);
				getInfo(&paraThread->destHost, tmpPort_DstPort, HelpDstAddress);
				firstpacket = 0;
			}


			if (logCheck != 0) {
				int net_TimeSec = ntohl(*(uint32_t *) ptrTimeSec); 
				int net_TimeUsec = ntohl(*(uint32_t *) ptrTimeUsec); 

				if (paraThread->payloadLogType == PL_STANDARD) {	
					writeInBufferStandard(&infos[paraThread->count], *(uint32_t *) payload,
							*(unsigned int *) ptrSeqNum, HelpSrcAddress, HelpDstAddress, tmpPort_SrcPort,
							tmpPort_DstPort, net_TimeSec, RcvTime.tv_sec % 86400L, net_TimeUsec,
							RcvTime.tv_usec, size);
				} else if (paraThread->payloadLogType == PL_SHORT) {	
					writeInBufferShort(&infos[paraThread->count], *(uint32_t *) payload,
							*(unsigned int *) ptrSeqNum, HelpSrcAddress, HelpDstAddress, tmpPort_SrcPort,
							tmpPort_DstPort, RcvTime.tv_sec % 86400L, RcvTime.tv_usec, size);
				} else {
					
					
					writeInBufferNone(&infos[paraThread->count], paraThread->flowId, HelpSrcAddress,
							HelpDstAddress, tmpPort_SrcPort, tmpPort_DstPort, RcvTime.tv_sec % 86400L,
							RcvTime.tv_usec, size);
				}
			}
			
			
			
			if (logRemote != 0) {
				int net_TimeSec = ntohl(*(uint32_t *) ptrTimeSec); 
				int net_TimeUsec = ntohl(*(uint32_t *) ptrTimeUsec); 

				if (paraThread->payloadLogType == PL_STANDARD) { 	
					writeInBufferStandard(&infos[paraThread->count], *(uint32_t *) payload,
							*(uint32_t *) ptrSeqNum, HelpSrcAddress, HelpDstAddress, tmpPort_SrcPort,
							tmpPort_DstPort, net_TimeSec, RcvTime.tv_sec % 86400L, net_TimeUsec,
							RcvTime.tv_usec, size); 						
				} else if (paraThread->payloadLogType == PL_SHORT) {	
					writeInBufferShort(&infos[paraThread->count], *(uint32_t *) payload, *(uint32_t *) ptrSeqNum,
							HelpSrcAddress, HelpDstAddress, tmpPort_SrcPort, tmpPort_DstPort,
							RcvTime.tv_sec % 86400L, RcvTime.tv_usec, size); 			
				} else {
					
					
					writeInBufferNone(&infos[paraThread->count], paraThread->flowId, HelpSrcAddress,
							HelpDstAddress, tmpPort_SrcPort, tmpPort_DstPort, RcvTime.tv_sec % 86400L,
							RcvTime.tv_usec, size);
				}
				infosHostToNet(&infos[paraThread->count]);
			}

			
			paraThread->count++;
			
			if (paraThread->count == logbuffer_size) {
				
				if (logCheck != 0)
					flushBuffer((ofstream *) paraThread->fileLog, infos, paraThread->count);
				
				else
				if (logRemote != 0)
						{
					MUTEX_THREAD_LOCK(mutexLogRem);

					if (sendto(paraThread->logSock, (char *) infos, paraThread->count * sizeof(struct info), 0,
							paraThread->logHost->ai_addr, paraThread->logHost->ai_addrlen) < 0)
						reportErrorAndExit("sctpSock", "sendto", "Cannot send log infos to LogServer");

					paraThread->count = 0;

					MUTEX_THREAD_UNLOCK(mutexLogRem);

					PRINTD(1,"sctpSock: Sent infos to LogServer\n");
				}
			} 
		} 

		
		if (paraThread->meter == METER_RTTM) {
			if (sendto(newSock, (char *) payload, size, 0, SrcAddress.ai_addr,
					SrcAddress.ai_addrlen) < 0)
				reportErrorAndExit("sctpSock", "sendto", "Cannot send payload back for rttm");
			PRINTD(2,"sctpSock: Sent RTTM infos\n");
		} 
	} 

	
	
	MUTEX_THREAD_LOCK(mutexSctp);
	if (--sctpSessionCount == 0)
		free(SrcAddress.ai_addr);
	MUTEX_THREAD_UNLOCK(mutexSctp);

	return NULL;
} 
#endif

#ifdef DCCP
void *dccpSock(void *param)
		{
	PRINTD(1,"dccpSock: dccpSock started\n");
	HANDLE hComm = 0;
	int sock = 0;
	int sendCheck = 0;
	unsigned char payload[MAX_PAYLOAD_SIZE];
	struct info *infos = (struct info *) calloc(logbuffer_size, sizeof(info));
	struct addrinfo SrcAddress;
	in_port_t tmpPort = 0;
	int newSock = 0;
	int size = 0;
	int optval = 0; 
	struct timeval RcvTime;

	unsigned char *ptrSeqNum = payload + sizeof(uint32_t);			
	unsigned char *ptrTimeSec = ptrSeqNum + sizeof(uint32_t);		
	unsigned char *ptrTimeUsec = ptrTimeSec + sizeof(uint32_t);		

	paramThread *paraThread;
	paraThread = (paramThread *) param;

	paraThread->addressInfos = infos;
	paraThread->count = 0;
	if (strcmp(paraThread->serial, "noSerial") != 0) {
		hComm = serialUp(paraThread->serial);
		if (hComm == INVALID_HANDLE_VALUE)
			printf("Error opening interface %s \n", paraThread->serial);
	}
	sock = socket(paraThread->destHost.ai_family, SOCK_DCCP, 0);
	if (sock < 0)
		reportErrorAndExit("dccpSock", "socket", "Cannot create a DCCP socket on port");

	if (paraThread->dsByte
			&& (paraThread->destHost.ai_family == AF_INET)
			&& setsockopt(sock, SOL_IP, IP_TOS, (char *) &paraThread->dsByte, sizeof(BYTE)) < 0) {
		printf("** WARNING ** Flow %d. Could not set DS byte to: %d\n", paraThread->flowId, paraThread->dsByte);
	}

	if (bind(sock, paraThread->destHost.ai_addr, paraThread->destHost.ai_addrlen) < 0) {
		printf(" Error into bind function\n");
		struct pipeMsg msg;
		msg.code = MSG_FT_ERR1;
		msg.flowId = paraThread->flowId;
		if (sendPipeMsg(paraThread->rPipe, &msg) < 0)
			printf(" sending msg error");
		sleep(INFINITE);
	}
	
	
	
	
	
	if (setsockopt(sock, SOL_DCCP, DCCP_SOCKOPT_SERVICE, &optval, sizeof(optval)) < 0)
		reportErrorAndExit("dccpSock", "setsockopt", "Cannot set option DCCP_SERVICE");
	
	if (listen(sock, SOMAXCONN) < 0)
		reportErrorAndExit("dccpSock", "listen", "Cannot listen on a port");
	else {
		
#ifdef WIN32
		int len=paraThread->destHost.ai_addrlen;
		getsockname(sock,paraThread->destHost.ai_addr,&len);
		paraThread->destHost.ai_addrlen=len;
#else
		getsockname(sock, paraThread->destHost.ai_addr, &(paraThread->destHost.ai_addrlen));
#endif
		GET_PORT((&(paraThread->destHost)), tmpPort);
		fprintf(stderr, "Listening on DCCP port : %d\n", ntohs(tmpPort));
		fflush(stderr);
		struct pipeMsg msg;
		msg.code = MSG_FT_OK;
		msg.flowId = paraThread->flowId;
		if (sendPipeMsg(paraThread->rPipe, &msg) < 0) {
			printf("error into sending msg to signal manager");
		}
	}
	SrcAddress.ai_family = paraThread->destHost.ai_family;
	if (SrcAddress.ai_family == PF_INET) {
		SrcAddress.ai_addr = (struct sockaddr *) malloc(sizeof(struct sockaddr_in));
		SrcAddress.ai_addrlen = sizeof(struct sockaddr_in);
	} else if (SrcAddress.ai_family == PF_INET6) {
		SrcAddress.ai_addr = (struct sockaddr *) malloc(sizeof(struct sockaddr_in6));
		SrcAddress.ai_addrlen = sizeof(struct sockaddr_in6);
	}

	pthread_cleanup_push(free, SrcAddress.ai_addr);

		if ((newSock = accept(sock, SrcAddress.ai_addr, (socklen_t *) &SrcAddress.ai_addrlen)) < 0)
			reportErrorAndExit("dccpSock", "accept", "Cannot accept connection");
		if (closeSock(sock) == -1)
			reportErrorAndExit("dccpSock", "closeSock", "Cannot close socket sock");
		paraThread->socketClose = newSock;
		int firstpacket = 1;
		char HelpSrcAddress[INET6_ADDRSTRLEN];
		char HelpDstAddress[INET6_ADDRSTRLEN];
		int tmpPort_SrcPort = 0;
		int tmpPort_DstPort = 0;

		TSTART(_tstart, secs, msecs, first, 0, RECEIVER);
		PRINTD(1,"dccpSock: main loop\n");
		while (1) {
			size = recv(newSock, (char *) payload, MAX_PAYLOAD_SIZE, 0);
			if (size < 0)
				reportErrorAndExit("udpSock", "recvfrom", "Cannot receive UDP packets");
			else if (size > 0) {
				PRINTD(2,"dccpSock: Received DCCP packet, size %d\n", size);
				if (hComm > 0) {
					DTR_Disable(hComm);
					DTR_Enable(hComm);
				}
				GET_TIME_OF_DAY(&RcvTime, _tend, _tstart, secs, msecs, 0, RECEIVER);
				if ((logCheck != 0) || (logRemote != 0)) {
					if (firstpacket == 1) {
						getInfo(&SrcAddress, tmpPort_SrcPort, HelpSrcAddress);
						getInfo(&paraThread->destHost, tmpPort_DstPort, HelpDstAddress);
						firstpacket = 0;
					}


					if (logCheck != 0) {
						int net_TimeSec = ntohl(*(uint32_t *) ptrTimeSec); 
						int net_TimeUsec = ntohl(*(uint32_t *) ptrTimeUsec); 

						if (paraThread->payloadLogType == PL_STANDARD) {	
							writeInBufferStandard(&infos[paraThread->count], *(uint32_t *) payload,
									*(unsigned int *) ptrSeqNum, HelpSrcAddress, HelpDstAddress,
									tmpPort_SrcPort, tmpPort_DstPort, net_TimeSec,
									RcvTime.tv_sec % 86400L, net_TimeUsec, RcvTime.tv_usec, size);
						} else if (paraThread->payloadLogType == PL_SHORT) {	
							writeInBufferShort(&infos[paraThread->count], *(uint32_t *) payload,
									*(unsigned int *) ptrSeqNum, HelpSrcAddress, HelpDstAddress,
									tmpPort_SrcPort, tmpPort_DstPort, RcvTime.tv_sec % 86400L,
									RcvTime.tv_usec, size);
						} else {
							
							
							writeInBufferNone(&infos[paraThread->count], paraThread->flowId,
									HelpSrcAddress, HelpDstAddress, tmpPort_SrcPort,
									tmpPort_DstPort, RcvTime.tv_sec % 86400L, RcvTime.tv_usec,
									size);
						}
					}
					
					
					
					if (logRemote != 0) {
						int net_TimeSec = ntohl(*(uint32_t *) ptrTimeSec); 	
						int net_TimeUsec = ntohl(*(uint32_t *) ptrTimeUsec); 	

						if (paraThread->payloadLogType == PL_STANDARD) {	
							writeInBufferStandard(&infos[paraThread->count], *(uint32_t *) payload,
									*(uint32_t *) ptrSeqNum, HelpSrcAddress, HelpDstAddress,
									tmpPort_SrcPort, tmpPort_DstPort, net_TimeSec,
									RcvTime.tv_sec % 86400L, net_TimeUsec, RcvTime.tv_usec, size); 		
						} else if (paraThread->payloadLogType == PL_SHORT) {	
							writeInBufferShort(&infos[paraThread->count], *(uint32_t *) payload,
									*(uint32_t *) ptrSeqNum, HelpSrcAddress, HelpDstAddress,
									tmpPort_SrcPort, tmpPort_DstPort, RcvTime.tv_sec % 86400L,
									RcvTime.tv_usec, size); 						
						} else {
							
							
							writeInBufferNone(&infos[paraThread->count], paraThread->flowId,
									HelpSrcAddress, HelpDstAddress, tmpPort_SrcPort,
									tmpPort_DstPort, RcvTime.tv_sec % 86400L, RcvTime.tv_usec,
									size);
						}
						infosHostToNet(&infos[paraThread->count]);
					}

					
					paraThread->count++;
					
					if (paraThread->count == logbuffer_size) {
						
						if (logCheck != 0)
							flushBuffer((ofstream *) paraThread->fileLog, infos, paraThread->count);
						
						else
						if (logRemote != 0) {
							MUTEX_THREAD_LOCK(mutexLogRem);

							if (sendto(paraThread->logSock, (char *) infos,
									paraThread->count * sizeof(struct info), 0,
									paraThread->logHost->ai_addr, paraThread->logHost->ai_addrlen)
									< 0)
								reportErrorAndExit("dccpSock", "sendto",
										"Cannot send log infos to LogServer");

							paraThread->count = 0;

							MUTEX_THREAD_UNLOCK(mutexLogRem);
							PRINTD(1,"dccpSock: Sent infos to LogServer\n");
						}
					}
				}
				if (paraThread->meter == METER_RTTM) {
					do {
						PRINTD(2,"dccpSock: Trying to send RTTM infos\n");
						sendCheck = sendto(newSock, (char *) payload, size, 0, SrcAddress.ai_addr,
								SrcAddress.ai_addrlen);
					} while (sendCheck < 0 && errno == EAGAIN);
					if (sendCheck < 0)
						reportErrorAndExit("dccpSock", "sendto", "Cannot send payload back for rttm");
					PRINTD(2,"dccpSock: Sent RTTM infos\n");
				}
			} 
		} 
		pthread_cleanup_pop(1);
	return NULL;
} 
#endif
