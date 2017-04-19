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
#include "../common/thread.h"
#include "ITGLog.h"
#include "channel.h"



void infosntoh(struct info *infos, unsigned int dim) {
	unsigned int count = 0;
	long int txTimeSec;
	long int txTimeUsec;
	long int rxTimeSec;
	long int rxTimeUsec;
	while (count < dim) {
		infos[count].flowId = ntohl(infos[count].flowId);
		infos[count].seqNum = ntohl(infos[count].seqNum);
		infos[count].srcPort = ntohl(infos[count].srcPort);
		infos[count].destPort = ntohl(infos[count].destPort);
		infos[count].size = ntohl(infos[count].size);
		txTimeSec = ntohl(infos[count].txTime1);
		txTimeUsec = ntohl(infos[count].txTime2);
		rxTimeSec = ntohl(infos[count].rxTime1);
		rxTimeUsec = ntohl(infos[count].rxTime2);
		infos[count].txTime1 = txTimeSec / 3600;
		infos[count].txTime2 = (txTimeSec % 3600) / 60;
		infos[count].txTime3 = txTimeSec % 60 + txTimeUsec /  1000000.0;

		infos[count].rxTime1 = rxTimeSec / 3600;
		infos[count].rxTime2 = (rxTimeSec % 3600) / 60;
		infos[count].rxTime3 = rxTimeSec % 60 + rxTimeUsec /  1000000.0;
		count++;
	}
}


void logPacketTCP(int newSockSignal, ofstream *out)
{
	
	int port;

	
	int newSockLog;

	
	struct sockaddr_in sockAddress;

	
	struct sockaddr_in srcAddress;

	

	
	int logSock, hold, hold2, dim2;

	
	struct info *infos = (struct info *) malloc(logbuffer_size * sizeof(info));
	char *infosOffset;

	
	socklen_t sinLen = sizeof(srcAddress);

	

	
	fd_set activeSet;

	int	dim_infos;

#ifdef DEBUG
	
    int numRecvdPkt=0;

	
    int numTrip=0;
#endif

	

	
	sockAddress.sin_family = AF_INET;
	
	sockAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	

	createDataChannel(sockAddress,newSockSignal,logSock,port,"TCP");

	
	if (listen(logSock, 5) < 0)
		reportErrorAndExit("logPacketTCP","listen","Error into listen on logSock");
	

	
	newSockLog = accept(logSock, (struct sockaddr *) &srcAddress, &sinLen);
	PRINTD(1,"logPacketTCP: newSockLog TCP : %d \n",newSockLog);
	if (newSockLog < 0)
		reportErrorAndExit("logPacketTCP","accept","Error into accept on logSock");

	
	while (1) {
		
		FD_ZERO(&activeSet);
		
		FD_SET((unsigned int)newSockSignal, &activeSet);
		
		FD_SET((unsigned int)newSockLog, &activeSet);

		
		if (select(FD_SETSIZE, &activeSet, NULL, NULL, 0) < 0)
			reportErrorAndExit("logPacketTCP","select",
			"Invalid file descriptor or operation interrupted by a signal - Close first Receiver");
		
		if (FD_ISSET(newSockSignal, &activeSet)) {
			
			break;
		
		} else if (FD_ISSET(newSockLog, &activeSet)) {
			
			
			dim2 = logbuffer_size * sizeof(struct info);
			hold = 0;
			infosOffset = (char *)infos;
			do {
				hold2 = recv(newSockLog, (char *)infosOffset, dim2, 0);
				hold += hold2;
				dim2 -= hold2;
				infosOffset += hold2;
			} while (((hold % sizeof(struct info)) != 0) && (hold2 >= 0));

			if (hold < 0)
				
				printf("** WARNING **  Data lost - Close First Receiver!\n");
			else {
				
				dim_infos = hold / sizeof(struct info);
				infosntoh(infos, dim_infos); 
				
#ifdef DEBUG
           			numRecvdPkt = numRecvdPkt + hold / sizeof(struct info);
				numTrip++;
#endif
				
				if (!(*out).write((char *) infos, hold))
					printf("** WARNING **  Can't write data!\n");
			}
		}
	}

#ifdef DEBUG
	
	char hostName[50];
	
	char hostIP[20];
	int rit1 = getnameinfo((sockaddr*)&senderLog,sizeof(senderLog),hostName, INET_ADDRSTRLEN, NULL, 0,
			NI_NOFQDN);
	int rit2 = getnameinfo((sockaddr*)&senderLog,sizeof(senderLog),hostIP, INET_ADDRSTRLEN, NULL, 0,
			NI_NUMERICHOST);
	if ((rit1 == 0) & (rit2 == 0))
		printf("Data transmission ended on TCP channel from %s(%s)\n",hostName,hostIP);
	else if ((rit1 != 0) & (rit2 == 0))
			printf("Data transmission ended on TCP channel from %s\n",hostIP);
	else
#endif
	printf("Data transmission ended on TCP channel!\n");
	fflush(stdout);

	
	free(infos);
	if (closeSock(logSock) < 0)
		reportErrorAndExit("logPacketTCP","closeSock","Cannot close logSock");
	if (closeSock(newSockLog) < 0)
		reportErrorAndExit("logPacketTCP","closeSock","Cannot close newLogSock");
	PRINTD(1,"logPacketTCP: Number of received packets : %d \n",numRecvdPkt);
	PRINTD(1,"logPacketTCP: Number of received infos : %d \n",numTrip);
}



void logPacketUDP(int newSockSignal, ofstream * out)
{
	
	int port;

	
	struct sockaddr_in sockAddress;

	

	
	int logSock, hold;

	
	struct info *infos = (struct info *) malloc(logbuffer_size * sizeof(info));

	

	
	fd_set  activeSet;

#ifdef DEBUG
	
    int numRecvdPkt=0;

	
    int numTrip=0;
#endif

	

	
	sockAddress.sin_family = AF_INET;
	
	sockAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	

	createDataChannel(sockAddress,newSockSignal,logSock,port,"UDP");

	

	while (1) {
		
		FD_ZERO(&activeSet);
		
		FD_SET((unsigned int)newSockSignal, &activeSet);
		
		FD_SET((unsigned int)logSock, &activeSet);


		
		if (select(FD_SETSIZE, &activeSet, NULL, NULL, 0) < 0)
			reportErrorAndExit("logPacketUDP","select",
			"Invalid file descriptor or operation interrupted by a signal - Close first Receiver");
		
		if (FD_ISSET(newSockSignal, &activeSet)) {
			
			break;
		
		} else if (FD_ISSET(logSock, &activeSet)) {
			
			hold = recv(logSock, (char *) infos, logbuffer_size * sizeof(struct info), 0);
			
			int dim_infos = hold / sizeof(struct info);
			infosntoh(infos, dim_infos); 
			
#ifdef DEBUG
			numRecvdPkt = numRecvdPkt + hold / sizeof(struct info);
			numTrip++;
#endif
			
			if (hold < 0)
				printf("** WARNING ** Data lost - Close First Receiver!\n");
			
			if (!(*out).write((char *) infos, hold))
				printf("** WARNING ** Can't write data!\n");
		}
	}

#ifdef DEBUG
	
	char hostName[50];
	
	char hostIP[20];
	int rit1 = getnameinfo((sockaddr*)&senderLog,sizeof(senderLog),hostName, INET_ADDRSTRLEN, NULL, 0,
			NI_NOFQDN);
	int rit2 = getnameinfo((sockaddr*)&senderLog,sizeof(senderLog),hostIP, INET_ADDRSTRLEN, NULL, 0,
			NI_NUMERICHOST);
	if ((rit1 == 0) & (rit2 == 0))
		printf("Data transmission ended on UDP channel from %s(%s)\n",hostName,hostIP);
	else if ((rit1 != 0) & (rit2 == 0))
			printf("Data transmission ended on UDP channel from %s\n",hostIP);
	else
#endif
	printf("Data transmission ended on UDP channel\n");
	fflush(stdout);

	
	free(infos);
	if (closeSock(logSock) < 0)
		reportErrorAndExit("logPacketUDP","closeSock","Cannot close logSock");
	PRINTD(1,"logPacketUDP: Number of received packets : %d \n",numRecvdPkt);
	PRINTD(1,"logPacketUDP: Number of received infos : %d \n",numTrip);
}



void createDataChannel(sockaddr_in sockAddress,int newSockSignal,int &logSock,int &port,
					   const char* protocolName)
{
	
	char msg[100];

	
	int size;
	unsigned int net_port;

	if (strcmp(protocolName, "TCP") == 0)
	{
		
		logSock = socket(PF_INET, SOCK_STREAM, 0);
		PRINTD(1,"createDataChannel: logSock TCP : %d \n", logSock);
		if (logSock < 0)
			reportErrorAndExit("createDataChannel","socket","Cannot create STREAM socket");
	}
	else
	{
		
		logSock = socket(PF_INET, SOCK_DGRAM, 0);
		PRINTD(1,"createDataChannel: logSock UDP : %d \n", logSock);
		if (logSock < 0)
			reportErrorAndExit("createdataChannel","socket","Cannot create DATAGRAM socket");
	}

	
	port = findPortFree(logSock);
	PRINTD(1,"createDataChannel: Port : %d \n", port);
	if (port == 1 )
		reportErrorAndExit("createDataChannel","findPortFree","Cannot find a port free");
	net_port = htonl(port); 
	
	memcpy(msg, &net_port, sizeof(int));
	
	size = sendto(newSockSignal, (const char *) &msg, sizeof(int), 0,
	    (struct sockaddr *) &senderLog, sizeof(senderLog));
	PRINTD(1,"createDataChannel: Size bytes sent: %d \n", size);
	if (size < 0)
		reportErrorAndExit("createDataChannel","sendto","Cannot send port on newSockSignal");
	printf("Receiving data on port: %d \n", port);
	fflush(stdout);
}
