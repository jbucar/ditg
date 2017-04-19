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





#define MSG_FT_OK 		1
#define MSG_FT_ERR1 	2
#define MSG_FT_ERR2 	3
#define MSG_FT_ERR_SOCK 4





struct paramThread {
	BYTE l7Proto;				
	struct addrinfo destHost;		
	int flowId;				
	char *iface;				
	info *addressInfos;			
	int count;				
	int socket;				
	int socketClose;		    	
	BYTE meter;				
	BYTE dsByte;				
	char *fileLog;
	int logSock;				
    struct addrinfo *logHost;
    char serial[DIM_NAME_SERIAL_INTERFACE];
#ifdef UNIX
	int rPipe[2];
#endif
#ifdef WIN32
	HANDLE rPipe[3];
#endif
	int preambleSize; 			
	int payloadLogType; 			

	uint16_t portForPssv;		
#ifdef MULTIPORT
	uint16_t indexPort;
#endif
};


struct memChannel {
	int flowId;				
	BYTE l4Proto;			
};


struct manageLogFile{
	ofstream out;				 
	int num;
	char logFile[DIM_LOG_FILE]; 
};

#ifdef MULTIPORT
typedef struct {
	int socket;
	short inUse;
# ifdef WIN32
	HANDLE mutexSharedSockets;
# endif
# ifdef UNIX
	pthread_mutex_t mutexSharedSockets;
# endif
} sharedSocketType;

extern sharedSocketType sharedTcpSockets[];
extern sharedSocketType sharedUdpSockets[];
#endif


extern const char DEFAULT_LOG_FILE[];


extern int flagTerm;


extern struct addrinfo *logHost;


extern int logCheck;


extern int logRemote;


extern BYTE l4ProtoLog;


extern ofstream out;


extern int sockSignaling;


extern char logFile[DIM_LOG_FILE];

extern addrinfo hint;

#ifdef WIN32

extern HANDLE mutexLog;

extern HANDLE mutexLogRem;

extern int userId;
#endif

#ifdef UNIX

extern pthread_mutex_t mutexLog;

extern pthread_mutex_t mutexLogRem;

extern uid_t userId;
#endif




void *signalManager(void *param);


void printHelpAndExit();



void terminate(int sign);


void reportErrorAndExit(const char *function, const char *program, const char *msg);


int sendAck(int signaling, BYTE typeMessage);


int sendAckFlow(int signaling,BYTE typeMessage,int flowId, uint16_t openedPort = 0);



void recvInit();



void parserRecv(int argc , char *argv[]);






#ifdef WIN32
int typeParser(BYTE type, int &numFlow, int newSockSignaling , memChannel flowIdNum[],
		  paramThread paraThread[], pthread_t hThr[], HANDLE rPipe[], char *fileLog, int logSock, int logSockSignaling,
          struct addrinfo *logHost);
#endif

#ifdef UNIX
int typeParser(BYTE type, int &numFlow, int newSockSignaling , memChannel flowIdNum[],
		  paramThread paraThread[], pthread_t hThr[], int rPipe[], char *fileLog, int logSock, int logSockSignaling,
          struct addrinfo *logHost);
#endif


#ifdef WIN32
void pipeParser(int newSockSignaling,int &numFlow, HANDLE rPipe[], memChannel flowIdNum[],
		   paramThread paraThread[], pthread_t hThr[]);
#endif

#ifdef UNIX
void pipeParser(int newSockSignaling,int &numFlow, int rPipe[], memChannel flowIdNum[],
		   paramThread paraThread[], pthread_t hThr[]);
#endif



void createRemoteLogFile(struct addrinfo &logHost, char logFile[DIM_LOG_FILE], BYTE protocolLog, int & logSockSignaling, int & logSock);



void createSignalingLogChannel(struct addrinfo logHost, char logFile[DIM_LOG_FILE], BYTE protocolLog, int & logSockSignaling);


void recvFlowLog(int newSockSignaling, struct addrinfo & logHost, BYTE &protocolLog, char logFile[]);





inline void flushBuffer(ofstream * pointerLog, struct info *infos, int &count)
{
	
	if (MUTEX_THREAD_LOCK(mutexLog) < 0)
		reportErrorAndExit("flushBuffer", "MUTEX_THREAD_LOCK", "Cannot lock mutex");

	if ((*pointerLog).is_open()){ 
		
		(*pointerLog).write((const char *) infos, count * sizeof(struct info));
		(*pointerLog).flush();	
		count = 0;
	}

	
	if (MUTEX_THREAD_UNLOCK(mutexLog) < 0)
		reportErrorAndExit("flusBuffer", "MUTEX_THREAD_UNLOCK", "Cannot unlock mutex");
}
void recvNameLog(char nameFile[DIM_LOG_FILE],int newSockSignaling);
void copia(const struct addrinfo* src, struct addrinfo & dst);

