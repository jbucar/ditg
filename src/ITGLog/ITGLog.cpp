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
#include "../common/timestamp.h"
#include "ITGLog.h"
#include "channel.h"

#ifdef WIN32
char nameProgram[]="ITGLog";
#endif




struct manageLogFile{
	ofstream out;				 
	int num;					 
	char logFile[DIM_LOG_FILE]; 
};


ofstream out;


int flagTerm = 0;

uint16_t logbuffer_size = 50;	


signaling signalingLog;


struct sockaddr_in senderLog;


int signalSocket = 0;


manageLogFile memLogFile[MAX_NUM_THREAD];


#ifdef UNIX
const char DEFAULT_LOG_FILE[] = "/tmp/ITGLog.log";

pthread_mutex_t mutexLog;
pthread_mutex_t mutexLogPort;
#endif

#ifdef WIN32
const char DEFAULT_LOG_FILE[] = "ITGLog.log";

HANDLE mutexLog;
#endif

void* waitStopKey(void* s)
{
#ifdef WIN32
	Sleep(1000);
#endif
	PRINTD(1, "waitStopKey: Waiting for the stop key\n");
	while (getchar() != 'C') {}
	printf("Terminated by request\n");
	terminate(0);
	return NULL;
}




#ifdef UNIX
#  define EXEC_NAME "./ITGLog  "
#endif
#ifdef WIN32
#  define EXEC_NAME "ITGLog.exe"
#endif
void printHelpAndExit()
{
	cout << "\nITGLog - Remote Log Server Component of D-ITG platform\n";

	cout << "\n Synopsis\n\n"

		"     "<<EXEC_NAME<<" [options]\n";

	cout << "\n Options\n\n"
		"     -h | --help            Display this help and exit\n\n"

		"     -q <log_buffer_size>   Number of packets to push to the log at once (default: " << logbuffer_size << ")\n";

	cout << "\nFor more information please refer to the manual.\n";

	exit(1);
}


void recvInfo(int signalingChannel, BYTE &protocol, char logFile[DIM_LOG_FILE])
{
	
	BYTE buffer = LOG_CONNECT;
	int size_r;
	
	if ( sendto(signalingChannel, (const char *) &buffer, sizeof(BYTE), 0,
	    (struct sockaddr *) &senderLog, sizeof(senderLog)) < 0)
		reportErrorAndExit("recvInfo","sendto","Cannot send buffer");
	PRINTD(1,"recvInfo: Sent LOG_CONNECT message\n");

	
	if ( (size_r = recv(signalingChannel, (char *) &signalingLog, sizeof(signalingLog), 0)) < 0)
		reportErrorAndExit("recvInfo","recv","Cannot receive signalingLog data");
	PRINTD(1,"recvInfo: Received infos on log activity\n");

	
	protocol = signalingLog.protocol;
	strcpy(logFile, signalingLog.logFile);
	printf("Name Log File : %s \n", logFile);
	printf("Protocol used : %s \n", invFindL4Proto(protocol));
	fflush(stdout);

}



char *allowedLogFile(char logFile[DIM_LOG_FILE])
{
	int i = 0;
	
	bool find = true;
	
	while ((i < MAX_NUM_THREAD) && (find == true)) {
		if (strcmp(memLogFile[i].logFile, logFile) == 0) {
			
			find = false;
			
			memLogFile[i].num++;
			return (char *) &memLogFile[i].out;
		} else
			i++;
	}
	i = 0;
	
	while (memLogFile[i].num != -1)
		i++;
	
	memLogFile[i].out.open(logFile, ios::out | ios::binary | ios::trunc);
	if (!memLogFile[i].out) {
		
		char* tail = (char *) malloc(sizeof("Error into open this file : ") + sizeof(logFile));
		if (tail == NULL)
			reportErrorAndExit("allowedLogFile","malloc","Insifficient memory available");
		
		sprintf(tail,"Error into open this file : %s",logFile);
		reportErrorAndExit("allowedLogFile","open",tail);
	}
	
	memLogFile[i].num = 1;
	strcpy(memLogFile[i].logFile, logFile);
	return (char *) &memLogFile[i].out;
}




void closeFileLog(ofstream * out)
{
	int i = 0;
	
	while (out != (ofstream *) & memLogFile[i].out)
		i++;
	
	memLogFile[i].num--;
	if (memLogFile[i].num == 0) {
		
 		memLogFile[i].num = -1;
		
		(*out).close();
		strcpy(memLogFile[i].logFile, " ");
	}
}



int findPortFree(int logSock)
{
	
	int exit = 1;
    
	int randomPort;
	
	int numTries;
    
	struct sockaddr_in sockAddress;
	
	sockAddress.sin_family = AF_INET;
	
	sockAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	numTries = 0;
	setSeedRandom();
	do {
		
		randomPort = rand() % OFFSET_LOG_PORT + SEED_LOG_PORT;
		PRINTD(1,"recvInfo: Random Port : %d \n", randomPort);

		
		sockAddress.sin_port = htons(randomPort);
		
		if (bind(logSock, (struct sockaddr *) &sockAddress, sizeof(sockAddress)) < 0) {
			PRINTD(1,"recvInfo: ** ERROR **  : Cannot bind a socket on port %d\n", randomPort);
			numTries++;
			exit = 1;
		} else
			exit = 0;
    
	} while ((exit == 1) && (numTries < OFFSET_LOG_PORT));
	
	if (numTries > OFFSET_LOG_PORT)
		return 1;
	else
		return randomPort;
}




void createSignalingChannel()
{
	struct sockaddr_in signalSock;		
	int sock_opt = 1;			
	int opt_len = sizeof(sock_opt);		

	
	signalSock.sin_family = AF_INET;
	signalSock.sin_port = htons(DEFAULT_LOG_PORT_SIGNALING);
#if defined UNIX
	signalSock.sin_addr.s_addr = htonl(INADDR_ANY);
#elif defined WIN32
	signalSock.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
#endif
	
	if ((signalSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		reportErrorAndExit("createSignalingChannel","socket","Cannot create socket signalSocket");
	}
	
	if (setsockopt(signalSocket, SOL_SOCKET, SO_REUSEADDR, (char *) &sock_opt, opt_len) < 0) {
		reportErrorAndExit("createSignalingChannel", "setsockopt", "Error setting socket option");
	}

	
	if (bind(signalSocket, (struct sockaddr *) &signalSock, sizeof(signalSock)) < 0) {
		reportErrorAndExit("createSignalingChannel","bind","Cannot bind socket signalSocket");
	}

	
	if (listen(signalSocket, SOMAXCONN) < 0) {
		reportErrorAndExit("createSignalingChannel","listen","Cannot listen on socket signalSocket");
	}
	PRINTD(1,"createSignalingChannel: TCP signaling channel created with signalSocket : %d\n",signalSocket);
}




void *channelManager(void *param)
{
	
	paramLogThread *para;
	
	para = (paramLogThread *) param;
	
	char *fileLog;
	
	char logFile[DIM_LOG_FILE];
	strcpy(logFile, DEFAULT_LOG_FILE);
	
	BYTE protocol;
	
	int newSockSignal = para->socket;
	
	recvInfo(newSockSignal, protocol, logFile);
	
	if ( MUTEX_THREAD_LOCK(mutexLog) < 0)
		reportErrorAndExit("channelManager","mutexThreadLock1","Cannot lock Log Mutex");
	
	fileLog = allowedLogFile(logFile);
	
	if ( MUTEX_THREAD_UNLOCK(mutexLog) < 0)
		reportErrorAndExit("channelManager","mutexThreadUnlock1","Cannot unlock Log Mutex");
	
	if (protocol == L4_PROTO_TCP)
		
		logPacketTCP(newSockSignal, (ofstream *) fileLog);
	
	else
		
		logPacketUDP(newSockSignal, (ofstream *) fileLog);
	
	if ( MUTEX_THREAD_LOCK(mutexLog) < 0)
		reportErrorAndExit("channelManager","mutexThreadLock2","Cannot lock Log Mutex");
	
	closeFileLog((ofstream *) fileLog);
	
	if ( MUTEX_THREAD_UNLOCK(mutexLog) < 0)
		reportErrorAndExit("channelManager","mutexThreadUnlock2","Cannot unlock Log Mutex");
	
	exitThread();
	if ( closeSock(newSockSignal) < 0)
		reportErrorAndExit("channelManager","closeSock","Cannot close sock newSockSignal");
	return 0;
}



void reportErrorAndExit(const char *function, const char *program, const char *msg)
{
	
	if (flagTerm == NO_TERMINATE)
	{
		fprintf(stderr, "\n** ERROR **\n");
		fprintf(stderr, "Function %s aborted caused by %s \n", function, program);
		fprintf(stderr, "** %s ** \n",msg);
		fflush(stderr);
		
		terminate(SIGTERM);
	}
	
	else if (flagTerm == ERROR_TERMINATE)
	{
		fprintf(stderr, "\n** ERROR IN TERMINATE **\n");
		fprintf(stderr, "Function %s aborted caused by %s \n", function, program);
		fprintf(stderr, "** %s ** \n",msg);
		fprintf(stderr, "Finish with error in terminate!\n");
		fflush(stderr);
		exit(1);
	}
	
	else
		sleep(INFINITE);
}



void terminate(int sign)
{
	if (flagTerm == NO_TERMINATE) {
		
		flagTerm = TERMINATE;
		PRINTD(1,"terminate: ** Terminate function ***\n");

		if ( MUTEX_THREAD_RELEASE(mutexLog) < 0) {
			
			flagTerm = ERROR_TERMINATE;
			reportErrorAndExit("terminate","MUTEX_THREAD_RELEASE","Cannot release Log Mutex");
		}
		
		if (signalSocket != 0) {
			if ( closeSock(signalSocket) < 0) {
				
				flagTerm = ERROR_TERMINATE;
				reportErrorAndExit("terminate","closeSock","Cannot close socket signalSocket");
			}
		}
#ifdef WIN32
		
		if ( WSACleanup() < 0){
			
			flagTerm = ERROR_TERMINATE;
			reportErrorAndExit("terminate","WSACleanup","Cannot clean WSA");
		}
#endif
		
		if (sign == SIGINT)
			printf("Finish with CRTL-C! \n");
		
		else if (sign == SIGTERM)
			printf("Finish requested caused by errors! \n");
		}
	exit(1);
}



void logInit()
{
	
#ifdef WIN32
	if ( InitializeWinsock(MAKEWORD(1,1)) != 1) {
		flagTerm = ERROR_TERMINATE;
		reportErrorAndExit("logInit","InitializeWinsock","Cannot initialize WinSocket");
	}
#endif
	if ( MUTEX_THREAD_INIT(mutexLog) < 0)
		reportErrorAndExit("logInit","CreateMutex","Cannot initialize Mutex");
}



void parserLog(int argc, char *argv[]) {
	
	if (argc > 0) {
		
		if (argv[0][0] == '-') {
			switch (argv[0][1]) {
			
			case 'h':
				printHelpAndExit();
				break;
			case 'q':
				if ((argc < 2) || (argv[1][0] == '-')){
					argc -= 1;
					argv += 1;
				} else {
					logbuffer_size = atoi(argv[1]);
					argc -= 2;
					argv += 2;
				}
				break;
			default:
				
				char* tail = (char *) malloc(sizeof("Unknown option ") + sizeof(argv[0][1]));
				if (tail == NULL)
					reportErrorAndExit("parserLog", "malloc1", "Insufficient memory available");
				
				sprintf(tail, "Unknow option : %s", argv[0]);
#ifdef UNIX
				printf("Type: ./ITGLog -h for help\n");
#endif
#ifdef WIN32
				printf("Type: ITGLog.exe -h for help\n");
#endif
				
				reportErrorAndExit("parserLog", "general parser", tail);
				break;
			}
			
		} else {
			
			char* tail = (char *) malloc(sizeof("Unknown option ") + sizeof(argv[0]));
			if (tail == NULL)
				reportErrorAndExit("parserLog", "malloc2", "Insufficient memory available");
			
			sprintf(tail, "Unknow option : %s", argv[0]);
#ifdef UNIX
			printf("Type: ./ITGLog -h for help\n");
#endif
#ifdef WIN32
			printf("Type: ITGLog.exe -h for help\n");
#endif
			
			reportErrorAndExit("parserLog", "general parser", tail);
		}
	}
}



int main(int argc, char *argv[])
{
   	
	pthread_t hThr[MAX_NUM_THREAD];

	
	paramLogThread para[MAX_NUM_THREAD];

	printVersion("ITGLog");

	

	pthread_t t1;
	if (argc > 1){
		if (strcmp(argv[1],"-gui") == 0){
			CREATE_THREAD((void*)"wait", &waitStopKey, NULL, t1, true);
			argc--;
			argv++;
		}
	}

	
	signal(SIGINT, terminate);

	
	for (int i = 0; i < MAX_NUM_THREAD; i++) {
		hThr[i] = 0;
		memLogFile[i].num = -1;
		strcpy(memLogFile[i].logFile, " ");
		para[i].socket = 0;
	}

	

	logInit();

	
	argv++;
	
	argc--;

	

	parserLog(argc,argv);

	
	int newSockSignal;

	
	socklen_t sinLen = sizeof(senderLog);

	
	createSignalingChannel();

	printf("Press Ctrl-C to terminate!\n");
	fflush(stdout);

	int i = 0;

	

	while (1) {
		newSockSignal = accept(signalSocket, (struct sockaddr *) &senderLog, &sinLen);
		if (newSockSignal < 0)
			reportErrorAndExit("main","accept","Cannot accept socket newSockSignal");
		else {
		
#ifdef DEBUG
			
			char hostName[50];
			
			char hostIP[20];
			int rit1 = getnameinfo((sockaddr*)&senderLog,sinLen,hostName, INET_ADDRSTRLEN, NULL, 0,
			    NI_NOFQDN);
            int rit2 = getnameinfo((sockaddr*)&senderLog,sinLen,hostIP, INET_ADDRSTRLEN, NULL, 0,
					NI_NUMERICHOST);
			if ((rit1 == 0) & (rit2 == 0))
				printf("Received %dth request of Remote Log from %s(%s)\n", i+1,	hostName,hostIP);
			else if ((rit1 != 0) & (rit2 == 0))
				printf("Received %dth request of Remote Log from %s\n",i+1,hostIP);
			else
				printf("Received %dth request of Remote Log \n",i+1);
#endif
		PRINTD(1,"main: newSockSignal : %d\n",newSockSignal);

		
		para[i].socket = newSockSignal;
		
		if ( CREATE_THREAD(&para[i], channelManager, NULL, hThr[i], true) < 0)
			reportErrorAndExit("main","createThread","Cannot create thread");
		PRINTD(2,"main: Return value CREATE_THREAD channelManager hThr[i] : %lu \n",(unsigned long int)hThr[i]);
		i++;
		}
	}
	return 0;
}
