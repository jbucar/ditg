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


 
#include "../common/thread.h"
#include "../libITG/ITGapi.h"
#include <stdio.h>
#include <string.h>
#include <cstdlib>

#ifdef UNIX
#include <pthread.h>
#endif

#include "ITGManager.h"

char host[] = "localhost";
char nameProgram[] = "ITGManager";


void Terminate(int sig)
{
#ifdef UNIX
	exit(1);
#endif
#ifdef WIN32
	ExitProcess(0);
#endif
}


void* waitStopKey(void* s)
{
	char c[10000];
	do {
		fgets(c, 10000, stdin);
		if (c[0] == 'C') break;
		if (c[0] == '-') DITGsend(host, c);
	}  while (c != 0);
	printf("Terminated by request\n");
	Terminate(0);
	return NULL;
}



int main(int argc, char* argv[])
{
	char *sender, *msg;
	int i;
	char command[10000] = "";

	if (argc > 1) if (strcmp(argv[1], "-h") == 0) {
		printf("\nITGManager 0.1 (c) 2007 Volker Semken\n\n");
		printf("Usage: ITGManager <host> <settings>\n\n");
		printf("   <host>       ITGSend Hostname or IP address\n");
		printf("   <settings>   Command line options used for ITGSend\n");
		Terminate(0);
	}

	printf("ITGManager started. Press Ctrl-C to terminate\n");
	pthread_t pkey;
	CREATE_THREAD((void*)"wait", &waitStopKey, NULL, pkey, true);

	if (argc > 1) strcpy(host, argv[1]);
	printf("Using \"%s\" as destination ITGSender\n", host);
	for (i=2;i<argc;i++) strcat(strcat(command, " "), argv[i]);
	if (argc > 2) DITGsend(host, command);

	do {
		if ((i = catchManagerMsg(&sender, &msg)) != MNG_NOMSG) {
			printf("Received msg type %d   %s   %s\n", i, sender, msg);
			fflush(stdout);
		}
#ifdef UNIX
		sleep(1);
#else
		Sleep(1);
#endif
	} while (1);
}
