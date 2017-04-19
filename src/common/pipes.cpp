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
#include <unistd.h>
#include "pipes.h"

#ifdef WIN32
int numPipes = 0;
HANDLE mutex_numPipes;
#endif

#ifdef UNIX
int sendPipeMsg(int fd[2], pipeMsg *msg)
{
	write(fd[1], msg, sizeof(pipeMsg));
	return(0);
}

int recvPipeMsg(int fd[2], pipeMsg *msg)
{
	read(fd[0], msg, sizeof(pipeMsg));
	return(0);
}

int createNewPipe(int fd[2])
{
	return(pipe(fd));
}

int closePipe(int fd[2])
{
	close(fd[1]);
	close(fd[0]);
	return(0);

}
#endif

#ifdef WIN32
int sendPipeMsg(HANDLE pipe[3], pipeMsg *msg)
{
	DWORD written = 0;
	DWORD err;
	HANDLE fd = pipe[0];
	HANDLE event = pipe[1];

	if (WriteFile(fd, msg, sizeof(pipeMsg), &written, NULL) == 0) {
                printf("Error writing in pipe, with written = %ld\n",written);
                err = GetLastError();
                printf("GetLastError returned %ld\n", err);
		return(-1);
	}
	if (written != sizeof(pipeMsg)) {
		printf("Error writing in pipe: written != sizeof(pipeMsg)\n");
		return(-1);
	}

	PRINTD(1,"sendPipeMsg: written: %d\n", (int) written);
	SetEvent(event);
	return(0);
}

int recvPipeMsg(HANDLE pipe[3], pipeMsg *msg)
{
	DWORD read = 0;
	DWORD err;

	
	if (ReadFile(pipe[2], msg, sizeof(pipeMsg) , &read , NULL) == 0) {
		printf("Error reading from pipe, with read = %ld\n", read);
                err = GetLastError();
                printf("GetLastError returned %ld\n", err);
		return(-1);
	}
	if (read != sizeof(pipeMsg)) {
		printf("Error reading from pipe: read != sizeof(pipeMsg)\n");
		return(-1);
	}

	return(0);
}


int createNewPipe(HANDLE pipe[3])
{
	int num;
	char pipename[30];
	HANDLE hpipe, namedPipe;
	WSAEVENT event;
	char stringa[30];
	
	MUTEX_THREAD_LOCK(mutex_numPipes);
	num = numPipes++;
	MUTEX_THREAD_UNLOCK(mutex_numPipes);

	
	strcpy(pipename,"\\\\.\\pipe\\");
	strcat(pipename, nameProgram);
	 sprintf(stringa, "%d", num);
	strcat(pipename, stringa);

	PRINTD(1,"createNewPipe: pipename: %s\n", pipename);

	
	namedPipe = CreateNamedPipe(
		(LPTSTR)pipename,			
		PIPE_ACCESS_DUPLEX,		
		PIPE_TYPE_BYTE |		
		PIPE_READMODE_BYTE |		
		PIPE_WAIT,			
		PIPE_UNLIMITED_INSTANCES,	
		PIPE_BUFSIZE,			
		PIPE_BUFSIZE,			
		PIPE_TIMEOUT,			
		NULL				
	);
	if (namedPipe == INVALID_HANDLE_VALUE) {
		printf("Error in CreateNamedPipe()\n");
		return (-1);
	}

	
	hpipe = CreateFile(pipename, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, 0, 0);
	if (hpipe == INVALID_HANDLE_VALUE) {
    		printf("Error in CreateFile()\n");
		return (-1);
	}

	
	event = CreateEvent(NULL, FALSE, FALSE, NULL);
	ResetEvent(event);

	pipe[0] = hpipe;  
	pipe[1] = event;  
	pipe[2] = namedPipe; 
	return(0);
}

int closePipe(HANDLE pipe[3])
{
	CloseHandle(pipe[2]);
	return 0;
}

#endif


