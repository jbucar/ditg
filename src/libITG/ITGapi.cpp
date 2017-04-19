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

static int socket_r = 0;

#ifdef WIN32
int InitializeWinsock(WORD wVersionRequested)
{
	WSADATA wsaData;
	int err;

	err = WSAStartup(wVersionRequested, &wsaData);

	
	
	if (err != 0)
		return 0;	

	
	
	
	

	
	
	
	
	

	
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
		return 0;

	
	return 1;
}


char nameProgram[]="ITGapi";
#endif



int DITGsend(char *sender, char *message)
{
#ifdef WIN32
	if (InitializeWinsock(MAKEWORD(1,1)) != 1)	{
		fprintf(stderr, "WSAStartup() failed");
		exit(1);
	}
#endif

	struct addrinfo* locale = 0;
	
	freeaddrinfo(locale);


	
	getaddrinfo("0.0.0.0", NULL, NULL, &locale);
	socket_r = socket(locale->ai_family, SOCK_DGRAM, 0);
	if (socket_r  < 0)
		printf("error into create a socket  - DITGSend function \n");	

#ifdef UNIX
	int flags;
	flags = fcntl(socket_r, F_GETFL, 0);
	fcntl(socket_r, F_SETFL, flags | O_NONBLOCK);
#endif

#ifdef WIN32
	unsigned long i = 1;
	ioctlsocket(socket_r, FIONBIO, &i);
#endif


	
	
	
	


	struct sockaddr_in Sender;
        struct hostent *host;

        Sender.sin_family = AF_INET;
        if (!(host = gethostbyname(sender))) {
                cerr << endl << "DITGsend: Invalid destination address" << endl;
                return -1;
        }
        memcpy((char *) &Sender.sin_addr, host->h_addr, host->h_length);
        Sender.sin_port = htons(DEFAULT_PORT_SENDER_MANAGER);
        if (sendto(socket_r, message, strlen(message), 0, (struct sockaddr *) &Sender,  sizeof(Sender)) != (int) strlen(message))
                return -1;
        return 0;
}


int catchManagerMsg(char **senderIP, char **msg)
{
        struct sockaddr_in Sender;
        socklen_t SenderSlen = sizeof(Sender);
        uint16_t length;
        uint8_t msgtype;
        char *buffer;

        if (!socket_r)
                return MNG_NOMSG;

        buffer = (char *) malloc(MAX_FLOW_LINE_SIZE);

        if (recvfrom(socket_r, buffer, MAX_FLOW_LINE_SIZE, 0, (struct sockaddr *) &Sender, &SenderSlen) < 0) {
                free(buffer);
                return MNG_NOMSG;
        }
        *senderIP = (char *) malloc(30);
        strncpy(*senderIP, inet_ntoa(Sender.sin_addr), 30);
        msgtype = *(uint8_t *) buffer;
        length = *((uint16_t *) &(buffer[1]));
        memmove(buffer, &buffer[3], length);
        buffer[length] = '\0';
        *msg = buffer;

        return msgtype;
}
