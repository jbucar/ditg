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




#ifdef WIN32
#	include <winsock2.h>
#	include <ws2tcpip.h>
#	include <iostream>
	
	typedef HANDLE pthread_t;
	extern char nameProgram[];

#endif

#ifdef UNIX
#	include <pthread.h>
#	include <iostream>
#	include <unistd.h>
#endif



#ifdef WIN32
	
#	define MUTEX_THREAD_LOCK(a) \
		mutexThreadLock(a)

	
#	define MUTEX_THREAD_UNLOCK(a) \
		mutexThreadUnlock(a)

	
#	define MUTEX_THREAD_RELEASE(a) \
		mutexThreadRelease(a)

	
#	define CREATE_THREAD(a,b,c,d,e) \
		createThread(a,b,c,d,e)
		
	
#	define MUTEX_THREAD_INIT(a) \
		mutexThreadInit(a)

	
#	define pthread_cleanup_push(a,b) 
#	define pthread_cleanup_pop(a)
#endif

#ifdef UNIX
	
#	define MUTEX_THREAD_LOCK(a) \
		mutexThreadLock(&a)

	
#	define MUTEX_THREAD_UNLOCK(a) \
		mutexThreadUnlock(&a)

	
#	define MUTEX_THREAD_RELEASE(a) \
		mutexThreadRelease(&a)

	
#	define CREATE_THREAD(a,b,c,d,e) \
		createThread(a,b,c,d,e)
		
	
#	define MUTEX_THREAD_INIT(a) \
		mutexThreadInit(&a)
#endif





int createThread(void *argument, void *(nameFunction) (void *), void *attrib, pthread_t& idThread, bool detach);


int joinThread(int numFlow, pthread_t hThr[]);


int terminateThread(pthread_t idThread);


void exitThread();



#ifdef WIN32

int mutexThreadInit(HANDLE &mutex);
#endif

#ifdef UNIX

int mutexThreadInit(void* mutex);
#endif


int mutexThreadRelease(void* mutex);


int mutexThreadLock(void* mutex);


int mutexThreadUnlock(void* mutex);




int closeSock(int socket);
