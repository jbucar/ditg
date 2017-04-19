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
    int num = 0;
#endif

#include "thread.h"
#include <stdio.h>
#include "debug.h"




int createThread(void *argument, void *(nameFunction) (void *), void *attrib, pthread_t &idThread, bool detach)
{
	
	int ret = 0;
#ifdef WIN32
	
	LPDWORD pid = 0;
	
	idThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) nameFunction, argument,
				(DWORD) NULL, pid);
	PRINTD(3,"createThread: Return value CreateThread (WIN32) %lu\n",(unsigned long int)idThread);
	if (idThread == NULL) ret = -1;
	PRINTD(3,"createThread: Return value CreateThread: %d\n",ret);
#endif

#ifdef UNIX
	

	ret = pthread_create(&idThread, NULL, nameFunction, argument);
	if (ret != 0) ret = -1;
	if (detach) pthread_detach(idThread);
	PRINTD(3,"createThread: Return value pthread_create (LINUX) %lu\n",(unsigned long int)idThread);
	PRINTD(3,"createThread: Return value createThread : %d\n",ret);
#endif
	return ret;
};


int terminateThread(pthread_t idThread)
{
	
	int ret = 0;
#ifdef WIN32
	
	if (idThread!=0) 
		ret = TerminateThread((HANDLE) idThread, 0);

	if (ret == 0) ret = -1;
	else ret = 0;
#endif
#ifdef UNIX
	
	if (idThread != 0)	
		ret = pthread_cancel(idThread);

	if (ret != 0)
		ret = -1;
#endif
	PRINTD(3,"CreateThread: Return value terminateThread : %d\n",ret);
	return ret;
};


void exitThread()
{
#ifdef WIN32
	
	ExitThread(0);
#endif
#ifdef UNIX
	
	pthread_exit(0);
#endif
};


int joinThread(int numFlow, pthread_t hThr[])
{
	int ret = 0;
#ifdef WIN32
	DWORD temp = 0;
	
	temp = WaitForMultipleObjects(numFlow, (const HANDLE *) hThr, TRUE, INFINITE);
    PRINTD(3,"joinThread: Return value WaitForMultipleObjects(WIN32) : %d\n",(int)temp);
	if (temp == WAIT_FAILED) ret = -1;
#endif
#ifdef UNIX
	
	int ret2 = 0;
	for(int i = 0; i < numFlow; i++) {
		ret2 = pthread_join(hThr[i], NULL);
		if (ret2 != 0)
			ret = -1;
	}
#endif
    PRINTD(3,"joinThread: Return value joinThread : %d\n",ret);
    return ret;
};



#ifdef WIN32

int mutexThreadInit(HANDLE &mutex)
{
	int ret = 0;
	
	char nameMutex[30] = "";

	
	snprintf(nameMutex, 30, "%s%d", nameProgram, ++num);

	mutex = CreateMutex(NULL, FALSE, nameMutex);

	PRINTD(3,"mutexThreadInit: Return value CreateMutex (WIN32) : %d \n",(int) mutex);
	PRINTD(3,"mutexThreadInit: Name mutex %s \n", nameMutex);

	if (mutex == NULL) ret = -1;
	PRINTD(3,"mutexThreadInit: Return value mutexThreadInit : %d \n",ret);
	return ret;
}
#endif

#ifdef UNIX

int mutexThreadInit(void* mutex)
{
	int ret = 0;
	
	ret = pthread_mutex_init((pthread_mutex_t *)mutex, NULL);
	PRINTD(3,"mutexThreadInit: Return value pthread_mutex_init (LINUX) : %p \n", mutex);
	PRINTD(3,"mutexThreadInit: Return value mutexThreadInit : %d \n",ret);
	return ret;
}
#endif


int mutexThreadLock(void *mutex)
{
	int ret = 0;
#ifdef UNIX
	
	ret = pthread_mutex_lock((pthread_mutex_t *) mutex);
	if (ret != 0 ) ret = -1;
#endif
#ifdef WIN32
	DWORD temp = 0;
	
	temp = WaitForSingleObject((HANDLE)mutex,INFINITE);
    PRINTD(3,"mutexThreadLock: Return value WaitForSingleObject(WIN32) : %d\n",(int)temp);
	if (temp == WAIT_FAILED) ret = -1;
#endif
    PRINTD(3,"mutexThreadLock: Return value mutexThreadLock : %d\n",ret);
	return ret;
};


int mutexThreadUnlock(void *mutex)
{
	int ret = 0;
#ifdef UNIX
	
	ret = pthread_mutex_unlock((pthread_mutex_t *) mutex);
	if (ret != 0 ) ret = -1;
#endif
#ifdef WIN32
	BOOL flag = 0;
	
	flag = ReleaseMutex((HANDLE)mutex);
    PRINTD(3,"mutexThreadUnlock: Return value ReleaseMutex(WIN32) in mutexThreadUnlock : %d\n",flag);
	if (flag == 0) ret = -1;
#endif
    PRINTD(3,"mutexThreadUnlock: Return value mutexThreadUnlock : %d\n",ret);
	return ret;
};



int mutexThreadRelease(void* mutex)
{
	int ret = 0;
#ifdef UNIX
	
	ret = pthread_mutex_destroy((pthread_mutex_t *)mutex);
	if (ret != 0) ret = -1;
#endif
#ifdef WIN32
		BOOL flag = 0;
		
		flag = CloseHandle((HANDLE)mutex);
		if (flag == 0) ret = -1;
#endif
    PRINTD(3,"mutexThreadRelease: Return value mutexThreadRelease : %d\n",ret);
	return ret;
}





int closeSock(int socket)
{
	int ret = 0;
#if defined UNIX
	
	ret = close(socket);
#endif
#ifdef WIN32
	
	ret = closesocket(socket);
	if (ret == SOCKET_ERROR) ret = -1;
#endif
    PRINTD(3,"closeSock: Return value closeSock : %d\n",ret);
	return ret;
};
