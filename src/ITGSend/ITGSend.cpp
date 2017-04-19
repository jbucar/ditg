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

#ifdef UNIX
#include <netdb.h>
#include "newran/newran.h"		
#include <sys/wait.h>
#include <math.h>
#ifdef SCTP
#include <netinet/sctp.h>
#endif
#include <netinet/tcp.h>	
#include <netinet/ip.h>
#include <sys/shm.h>
#include <pthread.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#endif

#ifdef WIN32
#include "newran/newran.h"	
#include <math.h>
#include <sys/timeb.h>
#include <time.h>
#include <signal.h>
#endif

#include "../common/thread.h"
#include "traffic.h"
#include "../common/pipes.h"
#include "ITGSend.h"
#include "../common/timestamp.h"
#include "../common/serial.h"

#ifdef UNIX
const char DefaultLogFile[] = "/tmp/ITGSend.log";
const char DefaultRecvLogFile[] = "/tmp/ITGRecv.log";
#endif
#ifdef WIN32
char DefaultLogFile[] = "ITGSend.log";
char DefaultRecvLogFile[] = "ITGRecv.log";
#endif
const char DefaultDestIP[] = DEFAULT_DEST_IP;	


flowDescriptor flows[MAX_NUM_THREAD];		
signalChannel signalChannels[MAX_NUM_THREAD];	
pthread_t h_flowParser[MAX_NUM_THREAD];		
flowParserParams parserParams[MAX_NUM_THREAD];	
int multiFlows = 0;
int flowCount = 0; 	
struct addrinfo *logHost = 0;
struct sockaddr_in ManagerIP;
socklen_t ManagerIPslen = sizeof(ManagerIP);
int isFirst = 1;
int isFirstThread = 1;
int isFirstThreadRttm = 1;
int signalChanCount = 0;
addrinfo hint;
char logFile[DIM_LOG_FILE];	
int logging = 0;		
uint16_t logbuffer_size = 50;	
int logremoto = 0;
int logSock;
int logSockSignaling;
int namelogReceiver = 0;
int payloadLogType = 0;        
int mean_adjustment = 1; 
bool setPriority = false;
ofstream out;			
BYTE protoTxLog = DEFAULT_PROTOCOL_TX_LOG;
BYTE protoTx_ServerLogReceiver = DEFAULT_PROTOCOL_TX_LOG_OPZ;
struct addrinfo *serverLogReceiver = 0;

char logFileReceiver[DIM_LOG_FILE];
int logServer = 0;
int managerMode;
bool cmdMode = false;
int managerSock;

bool passiveMode = false;				
bool okToNewSignalingChan = true;		
char globalSigAddr[INET6_ADDRSTRLEN];	

struct timeval RifTime;					
#ifdef WIN32
	LARGE_INTEGER _tstart_, _tend_;
	unsigned long _sec_ = 0, _msec_ = 0;
	int first = 1;
#endif

#ifdef UNIX

#ifdef SCTP
typedef union {
	struct sockaddr_in v4;
	struct sockaddr_in6 v6;
	struct sockaddr sa;
} sockaddr_storage_t;

sctpSession sctpSessions[MAX_NUM_THREAD];
pthread_mutex_t mutexSctp = PTHREAD_MUTEX_INITIALIZER;
#endif

pthread_mutex_t mutex;
pthread_mutex_t mutexLog;
pthread_mutex_t mutexErrLog;
pthread_mutex_t mutexLogRem;
pthread_mutex_t mutexBufferPayload;	
#endif
#ifdef WIN32
HANDLE mutex;
HANDLE mutexLog;
HANDLE mutexLogRem;
HANDLE mutexBufferPayload; 	
#endif
int termsock = 0;
pthread_t pkey;
char nameProgram[]="ITGSend";

void* waitStopKey(void* s)
{

#ifdef WIN32
	Sleep(1000);
#endif
	PRINTD(1, "waitStopKey: Waiting for the stop key\n");
	while (getchar() != 'C'){}
	termsock = 1;
	printf("Terminated by request\n");
	Terminate(0);
	return NULL;
}


int modeManager(int argc, char *argv[])
{

	flowCount = 0;	

	managerMode = 1;	
	multiFlows = 1;
	struct addrinfo* localHost = 0;
	int size_r = 0;
	int size_s = 0;
	char buffer[MAX_FLOW_LINE_SIZE];

	bool toFree;

	signal(SIGINT, Terminate);
	printf("Press Ctrl-C to terminate\n");
	fflush(stdout);

	argv++;
	argc--;
	parserMultiFlows(argv, argc);


	 getaddrinfo("0.0.0.0", NULL, &hint, &localHost);
	 managerSock = socket(localHost->ai_family, SOCK_DGRAM, 0);
	if (managerSock <0) {
			printf("error socket()!\n");
			fflush(stdout);
	}

	
	SET_PORT(localHost, htons(DEFAULT_PORT_SENDER_MANAGER));
	
	if (bind(managerSock, localHost->ai_addr, localHost->ai_addrlen)) {
			printf("error bind()!\n");
	}


	do {
		char *endline;
		size_r = recvfrom(managerSock, buffer, MAX_FLOW_LINE_SIZE-3, 0,(struct sockaddr *) &ManagerIP, &ManagerIPslen);
		PRINTD(1,"modeManager: Data received from SignalManager %d, \n",size_r);

		if ((endline = strstr(buffer,"\n")) != NULL) {
			size_r = (int)(endline - buffer);
		}
		buffer[size_r] = '\n';
		buffer[size_r + 1] = '\0';
		
		strncpy(parserParams[flowCount].line, buffer, MAX_FLOW_LINE_SIZE-3);
		
		parserParams[flowCount].flowId = flowCount + 1;

		
		memmove(buffer + 2 * sizeof(int), buffer, size_r);
		((uint8_t *) buffer)[0] = MNG_FLOWSTART;	
		((uint16_t *) buffer)[1] = size_r; 		

		size_s = sendto(managerSock, buffer, size_r + 3, 0, (struct sockaddr *) &ManagerIP, ManagerIPslen);
		if (size_s < 0) {
			perror("modeManager: error while sending ACK to the manager");
		}
		PRINTD(1,"modeManager: Notify ITGManager about the start of the generation %d, \n",size_s);

		CREATE_THREAD(&parserParams[flowCount], flowParser, NULL, h_flowParser[flowCount], true);
		if (h_flowParser[flowCount] < 0) {
			cerr << "Cannot create a process to handle flow " << flowCount +
			    1 << endl;
		}
		flowCount++;

		
		MUTEX_THREAD_LOCK(mutexBufferPayload);
		for (int j = 1; j <= flowCount; j++) {
			toFree = true;
			if (flows[j].ptrPayloadBuffer != NULL) {
				for (int k = 1; k <= flowCount; k++) {
					if ((flows[j].ptrPayloadBuffer == flows[k].ptrPayloadBuffer) && (j != k)) {
						toFree = false;
					}
				}
				if (toFree) {
					free(flows[j].ptrPayloadBuffer);
				}
			}
		}
		MUTEX_THREAD_UNLOCK(mutexBufferPayload);
	} while (1);
 return 0;
}


int modeScript(int argc, char *argv[])
{
	char temp[MAX_SCRIPT_LINE_SIZE];
	char line[MAX_SCRIPT_LINE_SIZE];

	flowCount = 0;
	multiFlows = 1;
	FILE *batch;

	if ((batch = fopen(argv[0], "r")) == NULL) {
		
		perror("ITGServer Main: ");
		memClean();
		cerr << "Cannot open '" << argv[0] << "' for reading" << endl;
		exit(1);
	}
	argv++;
	argc--;
	parserMultiFlows(argv, argc);
	
	if (fgets(line, sizeof(line) - 1, batch) == NULL) {
		cerr << "errore" << endl;
		memClean();
		exit(1);
	}

	
	strcpy(temp, line);
	do {
		if (strcmp(temp, "\n") > 0) {
			
			strcpy(parserParams[flowCount].line, temp);
			
			parserParams[flowCount].flowId = flowCount + 1;
			CREATE_THREAD(&parserParams[flowCount], flowParser, NULL, h_flowParser[flowCount], false);
			if (h_flowParser[flowCount] < 0) {
				perror("ITGSend Main: ");
				cerr << "Cannot create a process to handle flow " << flowCount + 1 << endl;
			}
			flowCount++;
		}
	} while (fgets(temp, sizeof(temp) - 1, batch));
	fclose(batch);


	
	joinThread(flowCount, h_flowParser);

	
	for (int j = 1; j <= flowCount; j++) {
		if (flows[j].ptrPayloadBuffer != NULL) {
			free(flows[j].ptrPayloadBuffer);
		}
	}

	return 0;
}


int modeCommandLine(int argc, char *argv[])
{
	PRINTD(1,"modeCommandLine: mode started\n");
	parserParams[0].flowId = 1;
	cmdMode = true;
	argvToString(argv, argc, parserParams[0].line);
	flowParser(&parserParams[0]);

	return (0);
}


int main(int argc, char *argv[])
{
	printVersion("ITGSend");
	
	strcpy(programName, argv[0]);
	argv++;
	argc--;
 	if (!argc) {
		printf("\nMissing argument!!!\nTry ITGSend -h or --help for more information\n");
		exit(1);
	} else if (((argv[0][0] == '-') && (argv[0][1] == 'h')) || ((argv[0][0] == '-')
		&& (argv[0][1] == '-') && (argv[0][2] == 'h'))) {
		printHelp();
		exit(1);
	}

	
	MUTEX_THREAD_INIT(mutex);
	MUTEX_THREAD_INIT(mutexLog);
	MUTEX_THREAD_INIT(mutexLogRem);
	MUTEX_THREAD_INIT(mutexBufferPayload);	

	
	memset(&hint, 0x00, sizeof(hint));
	hint.ai_family = AF_UNSPEC;

	
	TSTART( _tstart_, _sec_, _msec_, first, 1, 0);
	
	GET_TIME_OF_DAY(&RifTime, _tend_, _tstart_, _sec_, _msec_, 1, 0);
	PRINTD(1,"main: SignalingTime Time: %lu.%06lu sec\n", (long unsigned int) RifTime.tv_sec, (long unsigned int) RifTime.tv_usec);

	if (strcmp(argv[0],"-gui") == 0){
		
		CREATE_THREAD((void*)"wait", &waitStopKey, NULL, pkey, true);
	}

	strcpy(logFile, DefaultLogFile);
	strcpy(logFileReceiver, DefaultRecvLogFile);
#ifdef WIN32
	MUTEX_THREAD_INIT(mutex_numPipes);
	if (!InitializeWinsock(MAKEWORD(1, 1))) {
		printf("** ERROR ** WSAStartup() failed\n");
		exit(1);
	}
#endif

	for (int i = 0; i < MAX_NUM_THREAD; i++) {
		signalChannels[i].socket = -1;
		signalChannels[i].flows = 0;
		signalChannels[i].errorLog = false;

#ifdef SCTP
		sctpSessions[i].parsedStreams = 0;
#endif
	}

	
	if ((argc > 0) && (argv[0][0] == '-') && (argv[0][1] == 'Q'))
		modeManager(argc, argv);
	else if ((argc > 0) && (argv[0][0] != '-') && (findL7Proto(argv[0]) == LX_ERROR_BYTE)) {
		modeScript(argc, argv);
	} else {
		modeCommandLine(argc, argv);
	}

	
	Terminate(0);

} 


void parserMultiFlows(char *argv[], int argc)
{
	strcpy(logFile, DefaultLogFile);
	getaddrinfo(DefaultDestIP, NULL, &hint, &logHost);
	SET_PORT(logHost, htons(DEFAULT_LOG_PORT));
	protoTxLog = DEFAULT_PROTOCOL_TX_LOG;

	getaddrinfo(DefaultDestIP, NULL, &hint, &serverLogReceiver);
	SET_PORT(serverLogReceiver, htons(DEFAULT_LOG_PORT));
	protoTx_ServerLogReceiver = DEFAULT_PROTOCOL_TX_LOG_OPZ;

	while (argc > 0) {
		if (argv[0][0] == '-') {
			switch (argv[0][1]) {
			case 'h':
				printHelp();
				Terminate(0);
				break;
			case 'q':
				if ((argc < 2) || (argv[1][0] == '-') || (findL7Proto(argv[1]) != LX_ERROR_BYTE)){
					argc -= 1;
					argv += 1;
				} else {
					logbuffer_size = atoi(argv[1]);
					argc -= 2;
					argv += 2;
				}
				break;
			case 'l':
				logging = 1;
				if ((argc < 2) || (argv[1][0] == '-') || !findL7Proto(argv[1])) {
					strcpy(logFile, DefaultLogFile);
					argc -= 1;
					argv += 1;
				} else {
					strcpy(logFile, argv[1]);
					argc -= 2;
					argv += 2;
				}
				break;
			case 'L':
				logging = 1;
				logremoto = 1;
				
				if ((argc < 2) || (argv[1][0] == '-') ||
				    (findL7Proto(argv[1]) != LX_ERROR_BYTE)){
					argc -= 1;
					argv += 1;
				} else {    
					
					if (findL4Proto(argv[1]) == LX_ERROR_BYTE){
                   				if (logHost)
							freeaddrinfo(logHost);
						
						if (getaddrinfo(argv[1], NULL, &hint, &logHost))
							ReportErrorAndExit("General parser",
									   " Invalid log-server address or protocol", programName, 0);
						argc -= 1;
						argv += 1;
					} else {
						
						protoTxLog = findL4Proto(argv[1]);
            					argc -= 1;
						argv += 1;
					}  
					if ((argc >= 2) && (argv[1][0] != '-') && (findL7Proto(argv[1]) == LX_ERROR_BYTE)) {
						if (findL4Proto(argv[1]) == LX_ERROR_BYTE){
                        				if (logHost)
								freeaddrinfo(logHost);
							
							if (getaddrinfo(argv[1], NULL, &hint, &logHost))
								ReportErrorAndExit("General parser",
										   " Invalid log-server address", programName, 0);
							argc -= 2;
							argv += 2;
						} else {
							
							protoTxLog = findL4Proto(argv[1]);
            					        argc -= 2;
	       				                argv += 2;
						}
					} 
					else {
						argc -= 1;
						argv += 1;
					}
				} 
				break;
			case 'X':
				logServer = 1;
				
				if ((argc < 2) || (argv[1][0] == '-') ||
				    (findL7Proto(argv[1]) != LX_ERROR_BYTE)){
					argc -= 1;
					argv += 1;
				} else {    
					
					if (findL4Proto(argv[1]) == LX_ERROR_BYTE){
                   				if (serverLogReceiver)
							freeaddrinfo(serverLogReceiver);
						
						if (getaddrinfo(argv[1], NULL, &hint, &serverLogReceiver))
							ReportErrorAndExit("General parser",
									   " Invalid log-server address or protocol", programName, 0);
						argc -= 1;
						argv += 1;
					} else {
						
						protoTx_ServerLogReceiver = findL4Proto(argv[1]);
            					argc -= 1;
						argv += 1;
					}
					if ((argc >= 2) && (argv[1][0] != '-') && (findL7Proto(argv[1]) == LX_ERROR_BYTE)) {
						if (findL4Proto(argv[1]) == LX_ERROR_BYTE){
							if (serverLogReceiver)
								freeaddrinfo(serverLogReceiver);
							
							if (getaddrinfo(argv[1], NULL, &hint, &serverLogReceiver))
								ReportErrorAndExit("General parser",
										   " Invalid log-server address", programName, 0);
							argc -= 2;
							argv += 2;
						} else {
							
							protoTx_ServerLogReceiver = findL4Proto(argv[1]);
            					        argc -= 2;
	       				                argv += 2;
						}  
					} 
					else {
						argc -= 1;
						argv += 1;
					}
				} 
				break;
			case 'x':
				namelogReceiver = 1;
				if ((argc < 2) || (argv[1][0] == '-') || !findL7Proto(argv[1])){
					argc -= 1;
					argv += 1;
				} else {
					strcpy(logFileReceiver, argv[1]);
					argc -= 2;
					argv += 2;
				}
				break;
			default:
				char tempS[sizeof("Option ") + sizeof(argv[0][1])+sizeof(" can not be used as a global option in the multy flow mode !")];
				strcpy(tempS,"Option ");
				strcat(tempS,argv[0]);
				strcat(tempS, " can not be used as a global option in the multy flow mode !");
				ReportErrorAndExit("General parser", tempS,programName, 0);
				break;
			} 
		} 
	} 
}



void *flowParser(void *param)
{
	int argc;
	char *argv[200];
	char *token;
	flowParserParams *para = (flowParserParams *)param;
#ifdef WIN32
	struct _timeb tstruct;
#endif
	char *ManagerMsg = NULL;
	double seed = 0.0;

	int j = 0;
	int h = 0;
	int id = para->flowId;
	unsigned long int Delay = DefaultDelay;
	int chanId;
	struct pipeMsg msg;

	char *saveptr; 

	FILE * ptrFile = NULL; 	
	char line[30];			
	int lineNumber=1;		
	uint32_t tmpInt;		
	Real tmpFloat;			

	bool SigDestHostSpecify=false;	
	uint16_t SigPort=0;				

	PRINTD(1,"flowParser: started\n");
	
	flows[id].id = id;
	flows[id].iface = NULL;
	flows[id].meter = METER_OWDM;
	flows[id].l7Proto = LX_PROTO_NONE;
	flows[id].l4Proto = L4_PROTO_UDP;
	flows[id].DSByte = DefaultDSByte;
	flows[id].Duration = DefaultDuration;
	flows[id].icmptype = 8;
	flows[id].TTL = 0;
	flows[id].Nagle = true;
	flows[id].srcPortSpecify = false;
	flows[id].srcAddrSpecify = false;	
	flows[id].dstAddrSpecify = false;	
	flows[id].dstPortSpecify = false;	
	flows[id].minPayloadSize = StandardMinPayloadSize;
	flows[id].payloadLogType = payloadLogType;
	flows[id].mean_adjustment = mean_adjustment;

	strcpy(flows[id].serialReceiver,"noSerial");
	flows[id].serial = INVALID_HANDLE_VALUE;
	
	flows[id].ptrPayloadBuffer = NULL;      
	flows[id].payloadFile[0] = 0;        
	flows[id].dimPayloadBuffer = 0;         
	
	flows[id].pktSizeFile[0]='\0';		
	flows[id].vectSize.clear();			
	flows[id].timeFile[0]='\0';			
	flows[id].vectTime.clear();			
	flows[id].numPacket=0;				

	flows[id].KBToSend=0;				

	flows[id].SigSrcHost=NULL;			
	flows[id].SigSrcPort=0;				
	flows[id].SigInterface=NULL;		

	flows[id].pollingMode = false;		

	
	if (para->line[strlen(para->line) - 1] == '\n') {
		para->line[strlen(para->line) - 1] = '\0';
	} else {
		cerr << " No newline at the end of file" << endl;
		memClean();
		exit(1);
	}

	if (managerMode) {
		ManagerMsg = (char *) malloc(sizeof(char) * strlen(para->line) + sizeof(int) * 2);
		strcpy(ManagerMsg, para->line);
	}

	
	getaddrinfo(DefaultDestIP, NULL, &hint, &flows[id].DestHost);
	SET_PORT(flows[id].DestHost, htons(DEFAULT_PORT));

	
	getaddrinfo(DefaultDestIP, NULL, &hint, &flows[id].SrcHost);

	getaddrinfo(DefaultDestIP, NULL, &hint, &flows[id].SigDestHost);				
	SET_PORT(flows[id].SigDestHost, htons(DEFAULT_PORT_SIGNALING));				

	
	if (!multiFlows) {
		strcpy(logFile, DefaultLogFile);
		getaddrinfo(DefaultDestIP, NULL, &hint, &logHost);
		SET_PORT(logHost, htons(DEFAULT_LOG_PORT));
		protoTxLog = DEFAULT_PROTOCOL_TX_LOG;

		getaddrinfo(DefaultDestIP, NULL, &hint, &serverLogReceiver);
		SET_PORT(serverLogReceiver, htons(DEFAULT_LOG_PORT));
		protoTx_ServerLogReceiver = DEFAULT_PROTOCOL_TX_LOG_OPZ;
	}

	
	flows[id].IntArrivDistro = pdConstant;
	ConstantRV = new Constant(1);
	flows[id].IntArriv = new SumRandom(1000.0 / DefaultPktPerSec * (*ConstantRV));

	
	flows[id].PktSizeDistro = pdConstant;
	ConstantRV = new Constant(1);
	flows[id].PktSize = new SumRandom(DefaultPktSize * (*ConstantRV));

	
#ifdef BURSTY
	
	flows[id].bursty = false;
#endif
	


	

	token = strtok_r(para->line, cmdMode ? "^" : " ",&saveptr); 

	while (token != NULL) {
		argv[j] = token;
		PRINTD(1,"flowParser: TOKEN %d: %s\n", j, token);
		j++;
		token = strtok_r(NULL, cmdMode ? "^" : " ",&saveptr); 
	}
	argc = j;

	
	if (strcmp(argv[0],"-gui") == 0){
		argc -= 1;
		h += 1;
	}

	while (argc > 0) {
		PRINTD(1,"flowParser: Parsing option %c \n",argv[h][1]);

		if (argv[h][0] == '-') {
			char *tail;

			long int temp = 0;
			Real a, b;
			switch (argv[h][1]) {
			case 'p':
				if (argv[h][2] != 'o') {
					PRINTD(1,"flowParser: Parsing of p option \n");
					if ((argc < 2) || (argv[h+1][0] == '-'))
						ReportErrorAndExit("General parser", "Invalid payload log type", programName, 0);
					flows[id].payloadLogType = atoi(argv[h+1]);
					if ((flows[id].payloadLogType < 0) || (flows[id].payloadLogType > 2)) {
						printf("Warning !! Incorrect \"payload log type\" definition."
							"Admissible values are: \n 0 normal\n; 1 short\n; 2 none \n. Default value %d assumed",
							payloadLogType);
						flows[id].payloadLogType = payloadLogType;
					}
					PRINTD(1,"flowParser: Payload type %d \n",flows[id].payloadLogType);
					if ((flows[id].payloadLogType != 0) && (flows[id].meter == METER_RTTM ))
						ReportErrorAndExit("General parser", "It is possible to use the RTT meter only with the standard payload type", programName, 0);
					if (flows[id].payloadLogType == PL_SHORT)
						flows[id].minPayloadSize = ShortMinPayloadSize;

					else if (flows[id].payloadLogType == PL_NONE)
						flows[id].minPayloadSize = NoneMinPayloadSize;

					h += 2;
					argc -= 2;
				} else if (!strcmp(argv[h],"-poll")) {
					PRINTD(1,"flowParser: Polling mode for flow n° %d\n",id);
					flows[id].pollingMode=true;
					h += 1;
					argc -= 1;
				}else{
					ReportErrorAndExit("General parser", "Error in parsing \"p\" or \"-poll\" options", programName, id);
				}
				break;
			case 'm':
				PRINTD(1,"flowParser: Flow id %d \n  ", id);
				PRINTD(1,"flowParser: Meter option found\n");
				flows[id].meter = findMeter(argv[h + 1]);
				if (flows[id].meter == 255) {
					ReportErrorAndExit("General parser", "Invalid measure type", programName, 0);
				}
				if ((flows[id].meter == METER_RTTM) && (flows[id].minPayloadSize < StandardMinPayloadSize)) {
					printf("%d != %d ", flows[id].minPayloadSize, StandardMinPayloadSize);
					ReportErrorAndExit("General parser", "It is possible to use the RTT meter only with the standard payload type",
							programName, 0);
				}
				h += 2;
				argc -= 2;
				break;
			case 'a':
			{
				in_port_t tmp_port=0;
				if (flows[id].DestHost) {
					GET_PORT(flows[id].DestHost, tmp_port);
					freeaddrinfo(flows[id].DestHost);
				}
				if ((argc < 2) || getaddrinfo(argv[h + 1], NULL, &hint, &flows[id].DestHost))
						ReportErrorAndExit("General parser", "Invalid destination address", programName, id);
#ifndef NOIPV6
				if (flows[id].DestHost->ai_family == PF_INET6
						&& IN6_IS_ADDR_LINKLOCAL( &((struct sockaddr_in6 *) (flows[id].DestHost->ai_addr))->sin6_addr) 
						&& ((struct sockaddr_in6 *) (flows[id].DestHost->ai_addr))->sin6_scope_id == 0) {
					printf("** WARNING **: IPv6 address scope id not specified by the sender\n");
				}
#endif
				
				
				if (SigDestHostSpecify == false){
					GET_PORT(flows[id].SigDestHost,SigPort);
					freeaddrinfo(flows[id].SigDestHost);
					if ((argc < 2) || getaddrinfo(argv[h + 1], NULL, &hint, &flows[id].SigDestHost))
							ReportErrorAndExit("General parser",
									   "Invalid destination address for the signal channel",
									   programName, id);

#ifndef NOIPV6
					if (flows[id].SigDestHost->ai_family == PF_INET6
							&& IN6_IS_ADDR_LINKLOCAL( &((struct sockaddr_in6 *) (flows[id].SigDestHost->ai_addr))->sin6_addr ) 
							&& ((struct sockaddr_in6 *) (flows[id].SigDestHost->ai_addr))->sin6_scope_id == 0) {
						printf("** WARNING **: IPv6 address scope id not specified by the sender (signal channel)\n");
					}
#endif
					SET_PORT(flows[id].SigDestHost,SigPort);
				}
				

				SET_PORT(flows[id].DestHost, tmp_port);
				flows[id].dstAddrSpecify = true;			
				h += 2;
				argc -= 2;
				break;
			}
#ifdef UNIX
			case 'i':
			{
				if (flows[id].srcAddrSpecify == false){	
					if (argc < 2)
						ReportErrorAndExit("General parser", "Invalid interface", programName, id);
					flows[id].iface = (char*)malloc(strlen(argv[h + 1]) + 1);
					strcpy(flows[id].iface, argv[h + 1]);
					h += 2;
					argc -= 2;
				}else
					ReportErrorAndExit("General parser", "use -i <interface> OR -sa <source_address>", programName, id);	
				break;
			}
#endif
			case 'r':
				if (argv[h][2] == 'p') {
					if (argc < 2)
			        		ReportErrorAndExit("General parser", "Invalid port number", programName, id);
					SET_PORT(flows[id].DestHost, htons(atoi(argv[h + 1])));
					flows[id].dstPortSpecify=true;
					h += 2;
					argc -= 2;
				}
			        else if (argv[h][2] == 'k') {
					if (argc < 2)
						ReportErrorAndExit("General parser", "Invalid Serial Port", programName, id);
					strcpy(flows[id].serialReceiver, argv[h+1]);
					PRINTD(1,"flowParser: at Receiver Side %s is up \n", argv[h + 1]);
					h += 2;
					argc -= 2;
				}
				else {
					char tempS[sizeof("What is  ?") + sizeof(argv[h])];
					ReportErrorAndExit("General parser", strcat(strcat(strcpy(tempS, "What is "), argv[h]), " ?"), programName, id);
				}
				break;
			case 'f':
				if (argc < 2)
					ReportErrorAndExit("General parser", "Invalid TTL",
					    programName, id);
				temp = strtol(argv[h + 1], &tail, 0);
				strcpy(flows[id].serialReceiver,"noSerial");if ((tail == argv[h + 1]) || (temp < 0) || (temp > 255))
					ReportErrorAndExit("General parser", "Invalid TTL",
					    programName, id);
				flows[id].TTL = temp;
				h += 2;
				argc -= 2;
				break;
   			case 't':
				if ((argc < 2) || ((temp = strtol(argv[h + 1], NULL, 10)) <= 0))
					ReportErrorAndExit("General parser", "Invalid Duration",
					    programName, id);
				flows[id].Duration = temp;
				h += 2;
				argc -= 2;
				break;
			
	   		case 'z':
				if ((argc < 2) || ((temp = strtol(argv[h + 1], NULL, 10)) <= 0))
					ReportErrorAndExit("General parser", "Invalid number of packets",
						programName, id);
				flows[id].numPacket = temp;
				h += 2;
				argc -= 2;
			break;
			
			
			case 'k':
				if ((argc < 2) || ((flows[id].KBToSend = strtoul(argv[h + 1], NULL, 10)) <= 0))
					ReportErrorAndExit("General parser", "Invalid number of kbyte",
						programName, id);
				h += 2;
				argc -= 2;
				break;
			
			case 'd':
				if (argc < 2)
					ReportErrorAndExit("General parser", "Invalid Delay",
					    programName, id);
				temp = strtol(argv[h + 1], &tail, 10);
				if ((tail == argv[h + 1]) || (temp < 0))
					ReportErrorAndExit("General parser", "Invalid Delay",
					    programName, id);
				Delay = temp;
				h += 2;
				argc -= 2;
				break;
			case 'b':
				if (argc < 2)
					ReportErrorAndExit("General parser", "Invalid DS Byte",
					    programName, id);
				temp = strtol(argv[h + 1], &tail, 0);
				if ((tail == argv[h + 1]) || (temp < 0) || (temp > 255))
					ReportErrorAndExit("General parser", "Invalid DS Byte",
					    programName, id);
				flows[id].DSByte = temp;
				h += 2;
				argc -= 2;
				break;
			case 'j': 
				if (argc < 2)
					ReportErrorAndExit("General parser", "j option needs either 1 or 0",
					    programName, id);
				flows[id].mean_adjustment = atoi(argv[h+1]);
				if ((flows[id].mean_adjustment != 0) && (flows[id].mean_adjustment != 1))
					ReportErrorAndExit("General parser", "j option needs either 1 or 0",
					    programName, id);
				h += 2;
				argc -= 2;
				break;
			case 'L':
				if (multiFlows)
					ReportErrorAndExit("General parser", "-L option not allowed into script file", programName,
							id);
				logging = 1;
				logremoto = 1;
				
				if ((argc < 2) || (argv[h + 1][0] == '-') || (findL7Proto(argv[h + 1]) != LX_ERROR_BYTE)) {
					argc -= 1;
					h += 1;
				} else {    
					
					if (findL4Proto(argv[h + 1]) == LX_ERROR_BYTE) {
						if (logHost)
							freeaddrinfo(logHost);
						
						if (getaddrinfo(argv[h + 1], NULL, &hint, &logHost))
							ReportErrorAndExit("General parser", " Invalid log-server address or protocol",
									programName, id);
						argc -= 1;
						h += 1;
					} else {
						
						protoTxLog = findL4Proto(argv[h + 1]);
						argc -= 1;
						h += 1;
					}  
					if ((argc >= 2) && (argv[h + 1][0] != '-') && (findL7Proto(argv[h + 1]) == LX_ERROR_BYTE)) {
						if (findL4Proto(argv[h + 1]) == LX_ERROR_BYTE) {
							if (logHost)
								freeaddrinfo(logHost);
							
							if (getaddrinfo(argv[h + 1], NULL, &hint, &logHost))
								ReportErrorAndExit("General parser", " Invalid log-server address",
										programName, id);
							argc -= 2;
							h += 2;
						} else {
							
							protoTxLog = findL4Proto(argv[h + 1]);
							argc -= 2;
							h += 2;
						}  
					} 
					else {
						argc -= 1;
						h += 1;
					}
				} 
				break;
			case 'X':
				if (multiFlows)
					ReportErrorAndExit("General parser", "-X option not allowed into script file", programName,
							id);



				logServer = 1;
				
				if ((argc < 2) || (argv[h + 1][0] == '-') || (findL7Proto(argv[h + 1]) != LX_ERROR_BYTE)) {
					argc -= 1;
					h += 1;
				} else {    
					
					if (findL4Proto(argv[h + 1]) == LX_ERROR_BYTE) {
						if (serverLogReceiver)
							freeaddrinfo(serverLogReceiver);
						
						if (getaddrinfo(argv[h + 1], NULL, &hint, &serverLogReceiver))
							ReportErrorAndExit("General parser", " Invalid log-server address or protocol",
									programName, id);
						argc -= 1;
						h += 1;
					} else {
						
						protoTx_ServerLogReceiver = findL4Proto(argv[h + 1]);
						argc -= 1;
						h += 1;
					}  
					if ((argc >= 2) && (argv[h + 1][0] != '-') && (findL7Proto(argv[h + 1]) == LX_ERROR_BYTE)) {
						if (findL4Proto(argv[h + 1]) == LX_ERROR_BYTE) {
							if (serverLogReceiver)
								freeaddrinfo(serverLogReceiver);
							
							if (getaddrinfo(argv[h + 1], NULL, &hint, &serverLogReceiver))
								ReportErrorAndExit("General parser", " Invalid log-server address",
										programName, id);
							argc -= 2;
							h += 2;
						} else {
							
							protoTx_ServerLogReceiver = findL4Proto(argv[h + 1]);
							argc -= 2;
							h += 2;
						}  
					} 
					else {
						argc -= 1;
						h += 1;
					}
				} 
				break;
			case 'T':
				if (argc < 2)
					ReportErrorAndExit("General parser", "Invalid Protocol Type", programName, id);

				flows[id].l4Proto = findL4Proto(argv[h + 1]);
				PRINTD(1,"flowParser: Level 4 Protocol: %s\n", invFindL4Proto(flows[id].l4Proto));

				
				if (flows[id].l4Proto == LX_ERROR_BYTE)
					ReportErrorAndExit("General parser", "Invalid Protocol Type", programName, id);

				
				
					flows[id].minPayloadSize = flows[id].minPayloadSize + sizeof(int);

				
				if (flows[id].l4Proto == L4_PROTO_ICMP) {
					if ((argc > 2) && (argv[h + 2][0] != '-')) {
						flows[id].icmptype = atoi(argv[h + 2]);
						h += 1;
						argc--;
					}
				}
#ifndef DCCP
				if (flows[id].l4Proto == L4_PROTO_DCCP )
					ReportErrorAndExit("General parser", "Invalid protocol type.\nPlease, to generate DCCP traffic flows "
										"recompile D-ITG with the DCCP feature enabled  "
										"(see manual for more details).", programName, id);
#endif
#ifdef SCTP
				
				if (flows[id].l4Proto == L4_PROTO_SCTP) {
					int sctpId;
					int sctpStreams;

					if (argc < 4 || (argv[h + 2][0] == '-') || (argv[h + 3][0] == '-'))
						ReportErrorAndExit ("General parser", "SCTP protocol needs parameters (syntax: -T SCTP <association_id> <max_streams>)",
							programName, id);

					
					if (argv[h + 2][0] > '0' || argv[h + 2][0] < '9')
						sctpId = atoi(argv[h + 2]);
					else
						ReportErrorAndExit ("General parser", "Invalid SCTP id option", programName, id);

					
					if (argv[h + 3][0] > '0' || argv[h + 3][0] < '9')
						sctpStreams = atoi(argv[h + 3]);
					else
						ReportErrorAndExit ("General parser", "Invalid SCTP num_streams option", programName, id);
					


					
					if (sctpStreams > 1)
						ReportErrorAndExit ("General parser", "** SCTP multi-streaming support is not yet available **", programName, id);

					MUTEX_THREAD_LOCK(mutexSctp);
					
					if (sctpSessions[sctpId].parsedStreams == 0) {
						sctpSessions[sctpId].numStreams = sctpStreams;
						sctpSessions[sctpId].busyStreams = 0;
						sctpSessions[sctpId].parsedStreams = 1;
						sctpSessions[sctpId].sock = -1;
						PRINTD(1,"flowParser: Rilevata sessione SCTP (id = %d) con %d streams al suo interno\n", sctpId, sctpStreams);
					}
					else sctpSessions[sctpId].parsedStreams++;
					flows[id].sctpId = sctpId;
					PRINTD(1,"flowParser: Rilevato stream #%d della sessione SCTP %d\n", sctpSessions[sctpId].parsedStreams, sctpId);
					MUTEX_THREAD_UNLOCK(mutexSctp);

					if (sctpSessions[sctpId].parsedStreams > sctpSessions[sctpId].numStreams)
						ReportErrorAndExit ("General parser", "Too many streams defined into SCTP session", programName, id);

					h += 2;
					argc -= 2;
				}
#else
				if (flows[id].l4Proto == L4_PROTO_SCTP)
					ReportErrorAndExit("General parser", "Invalid protocol type.\nPlease, to generate SCTP traffic flows "
										"recompile D-ITG with the SCTP feature enabled "
										"(see manual for more details).", programName, id);
#endif
				h += 2;
				argc -= 2;
				break;
			case 's':
				if (strlen(argv[h]) == 2) {
					if ((argc < 2) || ((seed = strtod(argv[h + 1], NULL)) >= 1.0) || (seed <= 0.0))
						ReportErrorAndExit("General parser", "Invalid seed (0 < seed < 1)", programName, id);
					h += 2;
					argc -= 2;
				} else if (argv[h][2] == 'p') {
					if (argc < 2)
						ReportErrorAndExit("General parser", "Invalid port number", programName, id);
					SET_PORT(flows[id].SrcHost, htons(atoi(argv[h + 1])));
					flows[id].srcPortSpecify = true;
					h += 2;
					argc -= 2;
				} else if (argv[h][2] == 'k') {
					if (argc < 2)
						ReportErrorAndExit("General parser", "Invalid Serial Port", programName, id);
					flows[id].serial = serialUp(argv[h + 1]);
					if (flows[id].serial == INVALID_HANDLE_VALUE)
						printf("Error opening interface \n");
					strcpy(flows[id].serialReceiver, "noSerial");
					h += 2;
					argc -= 2;

					PRINTD(1,"flowParser: at Sender Side %s is up \n", argv[h + 1]);
				} else if (argv[h][2] == 'a') {	
					if (flows[id].iface == NULL) {
						in_port_t tmp_port=0;
						if (flows[id].SrcHost ) {
							GET_PORT(flows[id].SrcHost, tmp_port);
							freeaddrinfo(flows[id].SrcHost);
						}
						if ((argc < 2) || getaddrinfo(argv[h + 1], NULL, &hint, &flows[id].SrcHost ))
							ReportErrorAndExit("General parser", "Invalid source address",programName, id);
#ifndef NOIPV6
						if (flows[id].SrcHost->ai_family == PF_INET6
								&& IN6_IS_ADDR_LINKLOCAL( &((struct sockaddr_in6 *) (flows[id].SrcHost->ai_addr))->sin6_addr ) 
								&& ((struct sockaddr_in6 *) (flows[id].SrcHost->ai_addr))->sin6_scope_id == 0) {
							printf("** WARNING **: IPv6 address scope id not specified by the sender\n");
						}
#endif
						SET_PORT(flows[id].SrcHost, tmp_port);
						flows[id].srcAddrSpecify = true;

						h += 2;
						argc -= 2;
					} else {
						ReportErrorAndExit("General parser", "use -i <interface> OR -sa <source_address>", programName, id);
					}
				} else {	
					char tempS[sizeof("What is  ?") + sizeof(argv[h])];
					ReportErrorAndExit("General parser", strcat(strcat(strcpy(tempS, "What is "), argv[h]), " ?"),
							programName, id);
				}
				break;
			case 'l':
				if (multiFlows)
					ReportErrorAndExit("General parser", "-l option not allowed into script file", programName, id);
				logging = 1;
				if ((argc < 2) || (argv[h + 1][0] == '-') || !findL7Proto(argv[h + 1])) {
					strcpy(logFile, DefaultLogFile);
					argc -= 1;
					h += 1;
				} else {
					strcpy(logFile, argv[h + 1]);
					argc -= 2;
					h += 2;
				}
				break;
			case 'x':
				if (multiFlows)
					ReportErrorAndExit("General parser", "-x option not allowed into script file", programName, id);
	            		namelogReceiver = 1;
	            		if ((argc < 2) || (argv[h + 1][0] == '-') || !findL7Proto(argv[h + 1])){
					argc -= 1;
					h += 1;
				} else {
					strcpy(logFileReceiver, argv[h+1]);
					argc -= 2;
					h += 2;
				}
				break;
			case 'q':
				if ((argc < 2) || (argv[h + 1][0] == '-') || !findL7Proto(argv[h + 1])){
					argc -= 1;
					h += 1;
				} else {
					logbuffer_size = atoi(argv[h+1]);
					argc -= 2;
					h += 2;
				}
				break;
			case 'C':	
				if ((argc < 2) ||  (strtod(argv[h+1],NULL) <= 0.))
					ReportErrorAndExit("Protocol Parser",
					    "Invalid pkts per sec", programName, id);
				flows[id].IntArrivDistro = pdConstant;
				delete flows[id].IntArriv;
				ConstantRV = new Constant(1);
				flows[id].IntArriv =
				new SumRandom(1000.0 /  strtod(argv[h+1],NULL) * (*ConstantRV));
				h += 2;
				argc -= 2;
				break;
    			case 'U':
   				if ((argc < 3) ||  (strtod(argv[h+1],NULL) <= 0.)
				    ||  (strtod(argv[h+2],NULL) <= strtod(argv[h+1],NULL)) )
					ReportErrorAndExit("Protocol Parser",
					    "Invalid pkts per sec", programName, id);
				delete flows[id].IntArriv;
				flows[id].IntArrivDistro = pdUniform;
				a = (1000.0 / strtod(argv[h+2],NULL));
				b = (1000.0 / strtod(argv[h+1],NULL)) - a ;
				UniformRV = new Uniform;
				flows[id].IntArriv = new SumRandom( b * (*UniformRV) +  a);
				h += 3;
				argc -= 3;
				break;
			case 'E':
				if ((argc < 2) || (strtod(argv[h+1],NULL) <= 0.))
					ReportErrorAndExit("Protocol Parser",
					    "Invalid pkts per sec", programName, id);
				delete flows[id].IntArriv;
				flows[id].IntArrivDistro = pdExponential;
				ExponentialRV = new Exponential;
				flows[id].IntArriv =
				new SumRandom(1000.0 /  strtod(argv[h+1],NULL) * (*ExponentialRV));
				h += 2;
				argc -= 2;
				break;
			case 'V':
				if ((argc < 3) || (strtod(argv[h + 1], NULL) <= 0)
				    || (strtod(argv[h + 2], NULL) <= 0))
					ReportErrorAndExit("Protocol Parser",
					    "Invalid Pareto Distribution parameter values",
					    programName, id);
				a = (Real) strtod(argv[h + 1], NULL);
				b = (Real) strtod(argv[h + 2], NULL);
				ParetoRV = new Pareto(a);
				delete flows[id].IntArriv;
				flows[id].IntArrivDistro = pdPareto;
				flows[id].IntArriv = new SumRandom(b * (*ParetoRV));
				h += 3;
				argc -= 3;
				break;
			case 'Y':
				if ((argc < 3) || (strtod(argv[h + 2], NULL) <= 0))	
					ReportErrorAndExit("Protocol Parser",
					    "Invalid pkts per sec", programName, id);
				flows[id].IntArrivDistro = pdCauchy;
				b = strtod(argv[h + 1], NULL); 
				a = strtod(argv[h + 2], NULL); 
				delete flows[id].IntArriv;
				CauchyRV = new Cauchy;
				flows[id].IntArriv = new SumRandom(a * (*CauchyRV) + b);
				h += 3;
				argc -= 3;
				break;
			case 'N':
				if ((argc < 3) || (argv[h + 2] <= 0))	
					ReportErrorAndExit("Protocol Parser",
					    "Invalid pkts per sec", programName, id);
				flows[id].IntArrivDistro = pdNormal;
				b = strtod(argv[h + 1], NULL); 	
				a = strtod(argv[h + 2], NULL); 	
                		delete flows[id].IntArriv;
				NormalRV = new Normal;
				flows[id].IntArriv = new SumRandom(a * (*NormalRV) + b);
				h += 3;
				argc -= 3;
				break;
			case 'O':
				if ((argc < 2) || (strtod(argv[h + 1], NULL) <= 0))	
					ReportErrorAndExit("Protocol Parser",
					    "Invalid pkts per sec", programName, id);
				a = strtod(argv[h + 1], NULL);
				delete flows[id].IntArriv;
				
				PoissonRV = new Poisson(a);
				flows[id].IntArrivDistro = pdPoisson;
				flows[id].IntArriv = new SumRandom(1000.0 / (*PoissonRV));
				h += 2;
				argc -= 2;
				break;
			case 'G':
				if ((argc < 3) || (strtod(argv[h + 1], NULL) <= 0) || (strtod(argv[h + 2], NULL) <= 0))	
					ReportErrorAndExit("Protocol Parser",
					    "Invalid Gamma Distribution parameter values",
					    programName, id);
				a = (Real) strtod(argv[h + 1], NULL);
				b = (Real) strtod(argv[h + 2], NULL);
    				delete flows[id].IntArriv;
				
				GammaRV = new Gamma(a);
				flows[id].IntArrivDistro = pdGamma;
				flows[id].IntArriv = new SumRandom(b * (*GammaRV));
				h += 3;
				argc -= 3;
				break;
			   
			case 'W':
				if ((argc < 3) || (strtod(argv[h + 1], NULL) <= 0) || (strtod(argv[h + 2], NULL) <= 0))
					ReportErrorAndExit("Protocol Parser", "Invalid Weibull Distribution parameter values",
							programName, id);
				a = (Real) strtod(argv[h + 1], NULL);
				b = (Real) strtod(argv[h + 2], NULL);
				delete flows[id].IntArriv;
				WeibullRV = new Weibull(a, b);
				flows[id].IntArrivDistro = pdWeibull;
				flows[id].IntArriv = new SumRandom(1 * (*WeibullRV));
				h += 3;
				argc -= 3;
				break;
			   

			   
#ifdef BURSTY
			case 'B': 
				if ( argc < 3 )
					ReportErrorAndExit("Protocol Parser",
					    "Invalid settings for the On/Off periods1",
					    programName, id);
				h += 1;
				argc -= 1;
				burstyParser(h, argv, argc, flows[id].id, &flows[id].OnPeriod, &flows[id].OffPeriod,
				    flows[id].OnPeriodDistro, flows[id].OffPeriodDistro);
				flows[id].bursty = true;
				break;
#endif
			   
			case 'c':
				if ((argc < 2) || (atoi(argv[h + 1]) < 1))
					ReportErrorAndExit("Protocol Parser", "Invalid pkt size",
					    programName, id);
				delete flows[id].PktSize;
				flows[id].PktSizeDistro = pdConstant;
				ConstantRV = new Constant(1);
				flows[id].PktSize = new SumRandom(atoi(argv[h + 1]) * (*ConstantRV));
				h += 2;
				argc -= 2;
				break;
			case 'u':
				if ((argc < 3) || (atoi(argv[h + 1]) < 1)
				    || (atoi(argv[h + 2]) <= atoi(argv[h + 1])))
					ReportErrorAndExit("Protocol Parser", "Invalid pkt size",
					    programName, id);
				delete flows[id].PktSize;
				flows[id].PktSizeDistro = pdUniform;
	 			b = atoi(argv[h + 1]);
				a = atoi(argv[h + 2]) - b;
				UniformRV = new Uniform;
				flows[id].PktSize = new SumRandom(a * (*UniformRV) + b);
				argc -= 3;
				h += 3;
				break;
			case 'e':
				if ((argc < 2) || (atoi(argv[h + 1]) < 1))
					ReportErrorAndExit("Protocol Parser", "Invalid pkt size",
					    programName, id);
				delete flows[id].PktSize;
				flows[id].PktSizeDistro = pdExponential;
				ExponentialRV = new Exponential;
				flows[id].PktSize = new SumRandom(atoi(argv[h + 1]) * (*ExponentialRV));
				h += 2;
				argc -= 2;
				break;
			case 'v':
				if ((argc < 3) || (strtod(argv[h + 2], NULL) <= 0))	
					ReportErrorAndExit("Protocol Parser",
					    "Invalid Pareto Distribution parameter values",
					    programName, id);
				a = (Real) strtod(argv[h + 1], NULL);
				b = (Real) strtod(argv[h + 2], NULL);
				delete flows[id].PktSize;
				ParetoRV = new Pareto(a);
				flows[id].PktSizeDistro = pdPareto;
				flows[id].PktSize = new SumRandom(b * (*ParetoRV));
				argc -= 3;
				h += 3;
				break;
			case 'y':
				if ((argc < 3) || (atoi(argv[h + 2]) <= 0))	
					ReportErrorAndExit("Protocol Parser",
					    "Invalid pkts per sec", programName, id);
				delete flows[id].PktSize;
				flows[id].PktSizeDistro = pdCauchy;
				b = (Real) atoi(argv[h + 1]);	
				a = (Real) atoi(argv[h + 2]);	
				CauchyRV = new Cauchy;
				flows[id].PktSize = new SumRandom(a * (*CauchyRV) + b);
				h += 3;
				argc -= 3;
				break;
			case 'n':
				if ((argc < 3) || (argv[h + 2] <= 0))	
					ReportErrorAndExit("Protocol Parser", "Invalid pkt size",
					    programName, id);
				delete flows[id].PktSize;
				flows[id].PktSizeDistro = pdNormal;
				b = (Real) atoi(argv[h + 1]);	
				a = (Real) atoi(argv[h + 2]);	
				NormalRV = new Normal;
				flows[id].PktSize = new SumRandom(a * (*NormalRV) + b);
				h += 3;
				argc -= 3;
				break;
			case 'o':
				if ((argc < 2) || (atoi(argv[h + 1]) < 0))	
					ReportErrorAndExit("Protocol Parser", "Invalid pkt size",
					    programName, id);
				a = atoi(argv[h + 1]);
				delete flows[id].PktSize;
				
				PoissonRV = new Poisson(a);
				flows[id].PktSizeDistro = pdPoisson;
				flows[id].PktSize = new SumRandom(1 * (*PoissonRV));
				h += 2;
				argc -= 2;
				break;
			case 'g':
				if ((argc < 3) || (strtod(argv[h + 2], NULL) <= 0))
					ReportErrorAndExit("Protocol Parser",
					    "Invalid Gamma Distribution parameter values",
					    programName, id);
				delete flows[id].PktSize;
				a = (Real) strtod(argv[h + 1], NULL);
				b = (Real) strtod(argv[h + 2], NULL);
				
				GammaRV = new Gamma(a);
				flows[id].PktSizeDistro = pdGamma;
				flows[id].PktSize = new SumRandom(b * (*GammaRV));
				h += 3;
				argc -= 3;
				break;
			   
			case 'w':
				if ((argc < 3) || (strtod(argv[h + 1], NULL) <= 0) || (strtod(argv[h + 2], NULL) <= 0))
					ReportErrorAndExit("Protocol Parser", "Invalid Weilbull Distribution Parameter values",
							programName, id);
				a = (Real) strtod(argv[h + 1], NULL);
				b = (Real) strtod(argv[h + 2], NULL);
				WeibullRV = new Weibull(a, b);
				flows[id].PktSizeDistro = pdWeibull;
				flows[id].PktSize = new SumRandom(1 * (*WeibullRV));
				h += 3;
				argc -= 3;
				break;
			   
			case 'D':
				
				
				PRINTD(1,"flowParser: Nagle algorithm disabled\n");
				flows[id].Nagle=false;
				h += 1;
				argc -= 1;
				break;
#ifdef WIN32
			case 'P':
				
				
				if (SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS) == 0){
				        printf("Error - Impossible set priority class - %d \n", (int) GetLastError());
				        fflush(stdout);
			    }
				PRINTD(1,"flowParser: Enabled thread priority \n");
				setPriority = true;
				h += 1;
				argc -= 1;
				break;
#endif
			
			case 'H':
				PRINTD(1,"flowParser: Enabled passive mode \n");
				passiveMode = true;

				h += 1;
				argc -= 1;
				break;
			
			
			case 'F': 
				
				if (argv[h][2] == 'p') {
					PRINTD(1,"Parsing of Fp option \n");
					if ((argc < 2) || (argv[h+1][0] == '-'))
						ReportErrorAndExit("General parser", "Invalid name file specified for -Fp option",programName, id);

					
					strcpy(flows[id].payloadFile,argv[h + 1]);
					PRINTD(1,"File name of payload: %s \n",flows[id].payloadFile);

					if (multiFlows){
						PRINTD(1,"Parsing of F option in multiFlows mode \n");
						MUTEX_THREAD_LOCK(mutexBufferPayload);
						
						for(int j = 0 ; j < flowCount ; j++)
							
							if(flows[j].dimPayloadBuffer != 0 )
								
								if (strcmp(flows[id].payloadFile,flows[j].payloadFile)){
									
									flows[id].ptrPayloadBuffer = flows[j].ptrPayloadBuffer;
									flows[id].dimPayloadBuffer = flows[j].dimPayloadBuffer;
									PRINTD(1,"Parsing of F option: two identical file \n");
								}
						MUTEX_THREAD_UNLOCK(mutexBufferPayload);
					}
					if (flows[id].dimPayloadBuffer == 0){
						long int bytesRead = 0;

						
						ptrFile = fopen(flows[id].payloadFile,"r");
						
						if (ptrFile == NULL)
							ReportErrorAndExit("flowParser","Error in opening the file of payload", programName, id);
						
						fseek(ptrFile,0,SEEK_END);
						flows[id].dimPayloadBuffer = ftell(ptrFile);
						PRINTD(1,"Size of file of payload: %u \n",(unsigned int)flows[id].dimPayloadBuffer);
						
						flows[id].ptrPayloadBuffer = malloc(flows[id].dimPayloadBuffer);
						
						if (flows[id].ptrPayloadBuffer == NULL)
							ReportErrorAndExit("flowParser","Non enough memory for load the file", programName, id);
						
						fseek(ptrFile,0,SEEK_SET);
						bytesRead = fread(flows[id].ptrPayloadBuffer,1,flows[id].dimPayloadBuffer,ptrFile);
						
						if((bytesRead != flows[id].dimPayloadBuffer) || ferror(ptrFile))
							ReportErrorAndExit("flowParser","Error in reading the file of payload", programName, id);
						else
							
							fclose(ptrFile);
					}
				} else
				
				if (argv[h][2] == 's') {
					if ((argc < 2) || (argv[h+1][0] == '-'))
						ReportErrorAndExit("General parser", "Invalid name file specified for -Fs option",programName, id);

					
					strcpy(flows[id].pktSizeFile,argv[h + 1]);
					PRINTD(1,"flowParser: File name of the packets sizes: %s\n",flows[id].pktSizeFile);

					
					ptrFile = fopen(flows[id].pktSizeFile,"r");
					
					if (ptrFile == NULL)
						ReportErrorAndExit("flowParser","Error in opening the file of the packets sizes", programName, id);
					
					fseek(ptrFile,0,SEEK_SET);

					
					if (fgets(line, sizeof(line) - 1, ptrFile) == NULL) {
						ReportErrorAndExit("flowParser","Error in reading the file of the packets sizes", programName, id);
					}
					lineNumber=1;

					do{
						if(strcmp(line,"\n")!=0){
							if((atoi(line) <= 0) || (atoi(line) > DIM_MAX_IP_PACKET)){
								printf("\nError in reading the file of the packets sizes: negative, too big or invalid number at line %d\n",lineNumber);
								ReportErrorAndExit("flowParser","Error in reading the file of the packets sizes: negative, too big or invalid number", programName, id);
							}
							tmpInt=atoi(line);
							PRINTD(4,"flowParser: Size read = %d ; value n° = %d\n",tmpInt,(int)flows[id].vectSize.size()+1);
							flows[id].vectSize.push_back(tmpInt);
						}
						lineNumber++;
					} while (fgets(line, sizeof(line) - 1, ptrFile));
					fclose(ptrFile);

				}else
				
				if (argv[h][2] == 't') {
					if ((argc < 2) || (argv[h+1][0] == '-'))
						ReportErrorAndExit("General parser", "Invalid name file specified for -Ft option",programName, id);

					
					strcpy(flows[id].timeFile ,argv[h + 1]);
					PRINTD(1,"flowParser: File name of the IDT: %s\n",flows[id].timeFile);

					
					ptrFile = fopen(flows[id].timeFile,"r");
					
					if (ptrFile == NULL)
						ReportErrorAndExit("flowParser","Error in opening the file of the IDT", programName, id);
					
					fseek(ptrFile,0,SEEK_SET);

					
					if (fgets(line, sizeof(line) - 1, ptrFile) == NULL) {
						ReportErrorAndExit("flowParser","Error in reading the file of the IDT", programName, id);
					}
					lineNumber=1;

					do{
						if(strcmp(line,"\n")!=0){
							if((strtod(line,NULL) <= 0) || (strtod(line,NULL) == HUGE_VAL)){
								printf("\nError in reading the file of the IDT: negative, too big or invalid number at line %d\n",lineNumber);
								ReportErrorAndExit("flowParser","Error in reading the file of the IDT: negative, too big or invalid number", programName, id);
							}
							tmpFloat=strtod(line,NULL);
							PRINTD(2,"flowParser:  Time read = %f ; value n° = %d\n",tmpFloat,(int)flows[id].vectTime.size()+1);
							flows[id].vectTime.push_back(tmpFloat);
						}
						lineNumber++;
					} while (fgets(line, sizeof(line) - 1, ptrFile));
					fclose(ptrFile);

					
					Delay = (long int) floor(flows[id].vectTime[0]);
				} else {
					
					char tempS[sizeof("What is  ?") + sizeof(argv[h])];
					ReportErrorAndExit("General parser", strcat(strcat(strcpy(tempS, "What is "), argv[h]), " ?"),
							programName, id);
				}
				argc -= 2;
				h += 2;
				break;
			
			
			case 'S': 
				if (argv[h][2] == 's') { 
					if (argv[h][3] == 'a') { 
						freeaddrinfo(flows[id].SigSrcHost);
						if ((argc < 2) || getaddrinfo(argv[h + 1], NULL, &hint, &flows[id].SigSrcHost))
							ReportErrorAndExit("General parser",
									"Invalid source address for the signal channel", programName,
									id);

#ifndef NOIPV6

						if (flows[id].SigSrcHost->ai_family == PF_INET6
								&& IN6_IS_ADDR_LINKLOCAL( &((struct sockaddr_in6 *) (flows[id].SigSrcHost->ai_addr))->sin6_addr ) 
								&& ((struct sockaddr_in6 *) (flows[id].SigSrcHost->ai_addr))->sin6_scope_id
										== 0) {
							printf(
									"** WARNING **: IPv6 address scope id not specified by the sender (signal channel)\n");
						}
#endif

						PRINTD(1,"\nflowParser: Signaling source address: %s\n",argv[h + 1]);
					} else if (argv[h][3] == 'p') { 
						if (argc < 2)
							ReportErrorAndExit("General parser", "Invalid port number", programName, id);
						int tmp = atoi(argv[h + 1]);
						if ((tmp < 1) || (tmp > 65535))
							ReportErrorAndExit("General parser", "Invalid port number", programName, id);
						SigPort = tmp;
						PRINTD(1,"\nflowParser: Signaling source port: %d\n",SigPort);
						flows[id].SigSrcPort = SigPort;
					} else {
						char tempS[sizeof("What is  ?") + sizeof(argv[h])];
						ReportErrorAndExit("General parser",
								strcat(strcat(strcpy(tempS, "What is "), argv[h]), " ?"), programName,
								id);
					}
				} else if (argv[h][2] == 'd') { 
					if (argv[h][3] == 'a') { 
						GET_PORT(flows[id].SigDestHost, SigPort);
						freeaddrinfo(flows[id].SigDestHost);
						if ((argc < 2) || getaddrinfo(argv[h + 1], NULL, &hint, &flows[id].SigDestHost))
							ReportErrorAndExit("General parser",
									"Invalid destination address for the signal channel",
									programName, id);

#ifndef NOIPV6
						if (flows[id].SigDestHost->ai_family == PF_INET6
								&& IN6_IS_ADDR_LINKLOCAL( &((struct sockaddr_in6 *) (flows[id].SigDestHost->ai_addr))->sin6_addr) 
								&& ((struct sockaddr_in6 *) (flows[id].SigDestHost->ai_addr))->sin6_scope_id == 0)
							printf("** WARNING **: IPv6 address scope id not specified by the sender (signal channel)\n");
#endif

						SigDestHostSpecify=true;
						PRINTD(1,"\nflowParser: Signaling destination address: %s\n",argv[h + 1]);
						SET_PORT(flows[id].SigDestHost,SigPort);
					} else if (argv[h][3] == 'p') { 
						if (argc < 2)
							ReportErrorAndExit("General parser", "Invalid port number", programName, id);
						int tmp=atoi(argv[h + 1]);
						if ((tmp < 1) || (tmp > 65535))
							ReportErrorAndExit("General parser", "Invalid port number", programName, id);
						SigPort=tmp;
						PRINTD(1,"\nflowParser: Signaling destination port: %d\n",SigPort);
						SET_PORT(flows[id].SigDestHost,htons(SigPort));
					} else {
						char tempS[sizeof("What is  ?") + sizeof(argv[h])];
						ReportErrorAndExit("General parser", strcat(strcat(strcpy(tempS, "What is "), argv[h]), " ?"), programName, id);
					}
				}
#ifdef UNIX
				else if (argv[h][2] == 'i'){
					if (argc < 2)
						ReportErrorAndExit("General parser", "Invalid interface", programName, id);
					flows[id].SigInterface = (char*)malloc(strlen(argv[h + 1]) + 1);
					strcpy(flows[id].SigInterface, argv[h + 1]);
					PRINTD(1,"\nflowParser: Interface for signaling: %s\n",flows[id].SigInterface);
					h += 2;
					argc -= 2;
				}
#endif
				else {
					
					char tempS[sizeof("What is  ?") + sizeof(argv[h])];
					ReportErrorAndExit("General parser", strcat(strcat(strcpy(tempS, "What is "), argv[h]), " ?"), programName, id);
				}
				argc -= 2;
				h += 2;
				break;
			
			default:
				char temp[sizeof("What is  ?") + sizeof(argv[h])];
				ReportErrorAndExit("General parser", strcat(strcat(strcpy(temp,
						"What is "), argv[h]), " ?"), programName, id);
				break;
			} 
		} else {
			flows[id].l7Proto = findL7Proto(argv[h]);
			switch (flows[id].l7Proto) {
			case L7_PROTO_TELNET:

				telnetParser(&flows[id].IntArriv, &flows[id].PktSize, flows[id].IntArrivDistro, flows[id].PktSizeDistro);
				flows[id].l4Proto = L4_PROTO_TCP;
				h++;
				argc--;
				break;
			case L7_PROTO_VOIP:

				voIPParser(h, argv, argc, flows[id].id, &flows[id].IntArriv, &flows[id].PktSize,
				    flows[id].IntArrivDistro, flows[id].PktSizeDistro);
#ifdef DCCP
				if ( flows[id].l4Proto != L4_PROTO_UDP && flows[id].l4Proto != L4_PROTO_DCCP)
#endif
					flows[id].l4Proto = L4_PROTO_UDP;
				break;
			case L7_PROTO_DNS:

				dnsParser(&flows[id].IntArriv, &flows[id].PktSize, flows[id].IntArrivDistro, flows[id].PktSizeDistro);
				h++;
				argc--;
				break;
			
			case L7_PROTO_CSATTIVA:
				CSParsera(&flows[id].IntArriv, &flows[id].PktSize, flows[id].IntArrivDistro, flows[id].PktSizeDistro);
				flows[id].l4Proto= L4_PROTO_UDP;                
				h++;
				argc--;
				break;
			case L7_PROTO_CSINATTIVA:
				CSParseri(&flows[id].IntArriv, &flows[id].PktSize, flows[id].IntArrivDistro, flows[id].PktSizeDistro);
				flows[id].l4Proto= L4_PROTO_UDP;                
				h++;
				argc--;
				break;
			case L7_PROTO_QUAKE:
				QuakeParser(&flows[id].IntArriv, &flows[id].PktSize, flows[id].IntArrivDistro, flows[id].PktSizeDistro);
				flows[id].l4Proto= L4_PROTO_UDP;                
				h++;
				argc--;
				break;
			case LX_ERROR_BYTE:
				char temp[sizeof("What is  ?") + sizeof(argv[h])];
				ReportErrorAndExit("General parser", strcat(strcat(strcpy(temp,	"What is "), argv[h]), " ?"), programName, id);
				break;
			} 
		} 
	} 
	PRINTD(1,"flowParser: Terminate Parser flow %d\n",id);

#ifdef WIN32
	if ((flows[id].l4Proto == L4_PROTO_ICMP) && (flows[id].DestHost->ai_family == PF_INET6)) {
		printf("Error: traffic ICMP with protocol IPv6 is not supported \n");
		memClean();
	exit(1);
	}
#endif
	
	if (seed == 0.0) {
#ifdef WIN32
		_ftime(&tstruct);
		seed = 0.49 * sin(tstruct.millitm) + 0.50;	
#endif
#ifdef UNIX
		struct timeval tv;	
		gettimeofday(&tv, NULL);
		seed = 0.49 * sin((double)tv.tv_usec) + 0.50;	
#endif
	}

	
	MotherOfAll urng(seed);              
	Random::Set(urng);                   


	
	if (createNewPipe(flows[id].parserPipe) < 0) {
		printf("Error in flowParser() trying to create a new pipe. [flow %d]\n", id);
		exitThread();
	}

	
	if ((passiveMode == true) && (multiFlows == 1) && (flows[id].dstAddrSpecify == true)){
		ReportErrorAndExit("flowParser","\"-a <address>\" option not work on passive-multiflow mode", programName, id);
	}

	
	if((passiveMode==true) && (((flows[id].l4Proto!=L4_PROTO_TCP) && (flows[id].l4Proto!=L4_PROTO_UDP)) || (flows[id].l7Proto!=LX_PROTO_NONE))){
		ReportErrorAndExit("flowParser","Passive Mode only with TCP or UDP protocol", programName, id);
	}

	
	if((passiveMode == true) && (multiFlows == 1) && ((flows[id].srcPortSpecify == false) || (flows[id].dstPortSpecify == false))){
		ReportErrorAndExit("flowParser","Passive-multiflow mode requires \"-sp\" and \"-rp\" options", programName, id);
	}

#ifndef MULTIPORT
	
	if((passiveMode == false) && (multiFlows == 1) && (flows[id].dstPortSpecify == false)){
		ReportErrorAndExit("flowParser","Multiflow mode requires \"-rp\" option", programName, id);
	}
#endif

	
	if(passiveMode == true) {
		if (flows[id].srcPortSpecify == false){
			SET_PORT(flows[id].SrcHost, htons(DEFAULT_PORT_SENDER));
			flows[id].srcPortSpecify = true;
		}

		if (flows[id].dstPortSpecify == false){
			SET_PORT(flows[id].DestHost, htons(DEFAULT_PORT));
			flows[id].dstPortSpecify = true;
		}
	}

	bool flag=flows[id].dstAddrSpecify;
	

	
	
	
 	
	
	int rit = identifySignalManager(id, &chanId, flows[id].SigDestHost,flows[id].dstAddrSpecify);     

	if (rit == -1) {
		printf("Error into function identifySignalManager() \n");
		exitThread();
	}

	
	
	
	
	if(flag!=flows[id].dstAddrSpecify){
		char *buffer;

		
		if (flows[id].SigDestHost->ai_family==PF_INET){
			buffer=new char[INET_ADDRSTRLEN];
			
			getnameinfo(flows[id].SigDestHost->ai_addr,flows[id].SigDestHost->ai_addrlen,buffer,INET_ADDRSTRLEN,NULL,0,NI_NUMERICHOST);
		}else{
			
			buffer=new char[INET6_ADDRSTRLEN];
			
			getnameinfo(flows[id].SigDestHost->ai_addr,flows[id].SigDestHost->ai_addrlen,buffer,INET6_ADDRSTRLEN,NULL,0,NI_NUMERICHOST);
		}
		PRINTD(1,"flowParser: address of the Receiver: %s\n",buffer);
		
		GET_PORT((flows[id].DestHost),SigPort);
		if (flows[id].DestHost) freeaddrinfo(flows[id].DestHost);
		if(getaddrinfo(buffer, NULL, &hint, &(flows[id].DestHost)) < 0){
			perror("flowParser getaddrinfo() (1)");
			exitThread();
		}
		SET_PORT((flows[id].DestHost),SigPort);

		delete[] buffer;
	}

	
	if((passiveMode==true) && (flows[id].srcAddrSpecify==false)){
		if (flows[id].SigDestHost->ai_family == PF_INET6){
			GET_PORT((flows[id].SrcHost),SigPort);
			PRINTD(1,"flowParser: freeaddrinfo(flows[id].SrcHost)\n");

#if (defined WIN32 && defined IPv6RECV) || defined UNIX || defined BSD
			if (flows[id].SrcHost)
				freeaddrinfo(flows[id].SrcHost);
			if(getaddrinfo("::", NULL, &hint, &(flows[id].SrcHost)) < 0){
				perror("flowParser getaddrinfo() (2)");
				exitThread();
			}

#ifndef NOIPV6
			if (IN6_IS_ADDR_LINKLOCAL( &((struct sockaddr_in6 *)(flows[id].SrcHost->ai_addr))->sin6_addr ) 
				&& ((struct sockaddr_in6 *)(flows[id].SrcHost->ai_addr))->sin6_scope_id == 0)
				printf("** WARNING **: IPv6 address scope id not specified by the sender\n");
#endif
#endif
			SET_PORT((flows[id].SrcHost),SigPort);
		}
	}
	


	if ((namelogReceiver == 1) || (logServer == 1)) {
		if (signalChannels[chanId].errorLog == true){
			printf("Error log file specified is already open \n");
  			isChannelClosable(chanId);
			if (multiFlows)
	   			exitThread();
			else return 0;
		}
	}

	flows[id].Delay = Delay;	


	
	msg.code = MSG_SM_NEWFLOW;
	msg.flowId = id;
	if (sendPipeMsg(signalChannels[chanId].pipe, &msg) < 0) {
		perror("flowParser sending msg");
		exitThread();
	}

	
	if (recvPipeMsg(flows[id].parserPipe, &msg) < 0) {
		perror("flowParser receiving msg");
		exitThread();
	}

	PRINTD(1,"flowParser: msg received from signal manager %d\n",msg.code);

	switch (msg.code) {
	case MSG_FP_END:
		break;
	case MSG_FP_ERR1:
		printf("Error at Receiver side \n");
		break;
	case MSG_FP_ERR2:
		printf("Error - FlowSender interrupted by an error\n");
		break;
	default:
		printf("Error undefined message received from signal manager\n");
		break;
	}
	printf("Finished sending packets of flow ID: %d\n\n", msg.flowId);
	fflush(stdout);

	

	if (managerMode) {
		
		int length = strlen(ManagerMsg);
		memmove(ManagerMsg + 2 * sizeof(int), ManagerMsg, length);
		((int *) ManagerMsg)[0] = MNG_FLOWEND;	
		((int *) ManagerMsg)[1] = length;	
		sendto(managerSock, ManagerMsg, length + sizeof(int) * 2, 0, (struct sockaddr *) &ManagerIP,
		    ManagerIPslen);
		PRINTD(1,"flowParser: Notify ITGManager about the end of the generation \n");
	}

	
   	closePipe(flows[id].parserPipe);


    

	if (multiFlows)
	   exitThread();
    return 0;
}

void *signalManager(void *id)
{
	int	chanId, sock, size;
	BYTE	type;
	uint32_t flowId;
	struct pipeMsg msg;
	uint16_t openedPort=0;
#ifdef UNIX
	int fd, maxfd;
	fd_set	rset;
#endif
#ifdef WIN32
	HANDLE fd, namedPipe;
	HANDLE	events[2];
	DWORD available = 0;
	
	unsigned long pending;
#endif

	PRINTD(1,"signalManager: signalManager() started\n");

	
	chanId = *(int *)id;
	sock = signalChannels[chanId].socket;
	fd = signalChannels[chanId].pipe[0];
#ifdef WIN32
	events[0] = WSACreateEvent();
	WSAEventSelect(sock, events[0], FD_READ);
	events[1] = signalChannels[chanId].pipe[1];
	namedPipe = signalChannels[chanId].pipe[2];
#endif

#ifdef UNIX
	
	maxfd = MAX(fd, sock) + 1;		
	FD_ZERO(&rset);
#endif

	
	for (;;) {
		
#ifdef UNIX
		FD_SET(sock, &rset);
		FD_SET(fd, &rset);
		if (select(maxfd + 1, &rset, NULL, NULL, NULL) == -1) {
			if (errno == EINTR)
				continue;
			printf("error during select in signalManager %d\n", chanId);
			exitThread();
		}
		
		if (FD_ISSET(sock, &rset)) {
#elif defined WIN32
		PRINTD(1,"signalManager: before waitformultiple...\n");
		
		WaitForMultipleObjects(2, (const HANDLE *)events, FALSE, INFINITE);
		PRINTD(1,"signalManager: out of waitformultiple...\n");
		
		
		ResetEvent(events[0]);
		
		pending = 0;
		ioctlsocket(sock, FIONREAD, &pending);
		PRINTD(1,"signalManager: pending: %lu\n", pending);
		while (pending > 0) {
#endif
			

			
			type=0xFF;	
			size = recv(sock, (char *) &type, sizeof(type), 0);
			if (size <= 0) {
				ReportErrorAndExit("signalManager","Receiver has shut down the connection gracefully",programName,0);
			}
			PRINTD(1,"signalManager: received type %d on socket\n", type);
			switch (type) {
			case TSP_ACK_CLOSED_FLOW:

				
				recv(sock, (char *) &flowId, sizeof(flowId), 0);
				flowId = ntohl(flowId);
				PRINTD(1,"signalManager: received ack closed flow %d\n", flowId);

				

				
				if ( isChannelClosable(chanId))  {
					
					msg.code = MSG_FP_END;
					msg.flowId = flowId;
					if (sendPipeMsg(flows[flowId].parserPipe, &msg) < 0) {
						perror("signalManager sending msg");
						exitThread();
					}
					exitThread();
				} else {
					
					msg.code = MSG_FP_END;
					msg.flowId = flowId;
					if (sendPipeMsg(flows[flowId].parserPipe, &msg) < 0) {
						perror("signalManager sending msg");
						exitThread();
					}
				}
				break;
			case TSP_ACK_SEND_FLOW:
				
				recv(sock, (char *) &flowId, sizeof(flowId), 0);
				flowId = ntohl(flowId);

				recv(sock, (char *) &openedPort, sizeof(openedPort), 0); 						
				if (openedPort != 0) {															
					SET_PORT((flows[flowId].DestHost),openedPort);								
					PRINTD(1,"signalManager: received opened port %d\n", ntohs(openedPort));	
				}


				
				CREATE_THREAD(&(flows[flowId]), flowSender, NULL, flows[flowId].handle, true);
#ifdef WIN32
				if (setPriority == true){
					if (SetThreadPriority(flows[flowId].handle, THREAD_PRIORITY_TIME_CRITICAL) == 0)
					printf("Error - Impossible set priority for thread - %d \n", (int) GetLastError());
 				}
#endif
				break;
			case TSP_DISCOVERY:
				sendType(sock, TSP_ACK_DISCOVERY);
				break;
			case TSP_ERR_MSG_2:
				recv(sock, (char *) &flowId, sizeof(flowId), 0);
				flowId = ntohl(flowId);
				printf("Error on bind to receiver side\n");
				
				if ( isChannelClosable(chanId))  {
					
					msg.code = MSG_FP_ERR1;
					msg.flowId = flowId;
					if ( sendPipeMsg(flows[flowId].parserPipe, &msg) < 0) {
						perror("signalManager sending msg");
						exitThread();
					}
			 		exitThread();
				} else {
					msg.code = MSG_FP_ERR1;
					msg.flowId = flowId;
					if ( sendPipeMsg(flows[flowId].parserPipe, &msg) < 0) {
						perror("signalManager sending msg");
						exitThread();
				 	}
				}
				break;
			case TSP_ERR_MSG_3:
				recv(sock, (char *) &flowId, sizeof(flowId), 0);
				flowId = ntohl(flowId);
				printf("** To generate ICMP traffic sender and receiver must be root\n");
				
				if (isChannelClosable(chanId))  {
					
					msg.code = MSG_FP_ERR1;
				 	msg.flowId = flowId;
				 	if (sendPipeMsg(flows[flowId].parserPipe, &msg) < 0) {
						perror("signalManager sending msg");
						exitThread();
				 	}
			 		exitThread();
				}else {
					
				 	msg.code = MSG_FP_ERR1;
				 	msg.flowId = flowId;
				 	if (sendPipeMsg(flows[flowId].parserPipe, &msg) < 0) {
						perror("signalManager sending msg");
						exitThread();
				 	}
				}
    			break;
    			
			case TSP_ERR_MSG_5:
				recv(sock, (char *) &flowId, sizeof(flowId), 0);
				flowId = ntohl(flowId);
				printf("** Connection closed unexpectedly\n");
				
				if (isChannelClosable(chanId))  {
					
					msg.code = MSG_FP_ERR1;
					msg.flowId = flowId;
					if (sendPipeMsg(flows[flowId].parserPipe, &msg) < 0) {
						perror("signalManager sending msg");
						exitThread();
					}
					exitThread();
				}else {
					
					msg.code = MSG_FP_ERR1;
					msg.flowId = flowId;
					if (sendPipeMsg(flows[flowId].parserPipe, &msg) < 0) {
						perror("signalManager sending msg");
						exitThread();
					}
				}
				break;
				
			default:
				ReportErrorAndExit("signalManager","Got unknown message from receiver",programName,0);
				break;
			} 
			fflush(stdout);
#ifdef WIN32
			ioctlsocket(sock, FIONREAD, &pending);
#endif
		} 

#ifdef UNIX
		
		if (FD_ISSET(fd, &rset)) {
#elif defined WIN32
		
		if (PeekNamedPipe(namedPipe, NULL , 0 , NULL , &available , NULL) == 0) {
			printf("Error in peek named pipe\n");
			exitThread();
		}
		PRINTD(1,"signalManager: available: %d\n", (int) available);

		while(available > 0) {
#endif
			
			if (recvPipeMsg(signalChannels[chanId].pipe, &msg) < 0) {
				perror("signalManager receiving msg");
				exitThread();
			}
			PRINTD(1,"signalManager: received msg code: %d\n", msg.code);
			switch(msg.code) {
			case MSG_SM_NEWFLOW:
				PRINTD(1,"signalManager: perform a new request to send \n");
				flows[msg.flowId].sigChanId = chanId;
			       	requestToSend(msg.flowId, sock);
				break;
			case MSG_SM_ENDFLOW:
				
				PRINTD(1,"signalManager: sending request on signalling channel to close flow #%d\n", msg.flowId);
				closedFlow(msg.flowId, sock);
				break;
			case MSG_SM_ERRFLOW:
				
				PRINTD(1,"signalManager: recover from a receiver error notification \n");
				msg.code = MSG_FP_ERR2;
				if (sendPipeMsg(flows[msg.flowId].parserPipe, &msg) < 0) {
					perror("signalManager sending msg");
					exitThread();
				}
				closedFlowErr(msg.flowId, sock);
				
				if ( isChannelClosable(chanId))
					exitThread();
				break;
			
			case MSG_SM_SNDACK:
				PRINTD(1,"signalManager: sendAckFlow \n");
				if(sendAckFlow(msg.flowId, sock)<0){
					perror("signalManager sending msg (MSG_SM_SNDACK)");
					exitThread();
				}
				break;
			
			default:
				printf("Got unknown message from queue\n");
				fflush(stdout);
				break;
			}
#ifdef WIN32
			
			if (PeekNamedPipe(fd, NULL , 0 , NULL , &available , NULL) == 0) {
				printf("Error in peek named pipe\n");
				exitThread();
			}
#endif
		} 

	} 
	return NULL;	
}



int isChannelClosable(int id)
{
	MUTEX_THREAD_LOCK(mutex);
	signalChannels[id].flows--;
	if (signalChannels[id].flows == 0) {
		sendRelease(signalChannels[id].socket);

		if (closeSock(signalChannels[id].socket) < 0){
			printf("error closing socket\n");
			fflush(stdout);
		}
		signalChannels[id].socket = -1;
		signalChannels[id].errorLog = false;

		closePipe(signalChannels[id].pipe);

  		MUTEX_THREAD_UNLOCK(mutex);
		PRINTD(1, "isChannelClosable: closing signaling channel\n");
		return(1);
	}
	MUTEX_THREAD_UNLOCK(mutex);
	return(0);
}


int identifySignalManager(int flowId, int *chanId, struct addrinfo * &DestHost,bool &dstAddrSpecify) 	
{
#ifdef UNIX
	int signalSock;
#else
	unsigned int signalSock;
#endif
	char type =0;
	bool checkChanID = false;

	PRINTD(1,"identifySignalManager: flowId=%d\n",flowId);

	MUTEX_THREAD_LOCK(mutex);

	checkChanID = checkDestHostIP(chanId, DestHost);

	
	
	if ((checkChanID == false) && (okToNewSignalingChan == true)) {												
		PRINTD(1,"identifySignalManager: Open a new signaling channel, chanId=%d, flowId=%d\n",*chanId,flowId);	
		
		
#ifdef UNIX
		createTransportChan(signalSock, DestHost,dstAddrSpecify,flows[flowId].SigSrcHost,flows[flowId].SigSrcPort,flows[flowId].SigInterface);		   
#else
		createTransportChan(signalSock, DestHost,dstAddrSpecify,flows[flowId].SigSrcHost,flows[flowId].SigSrcPort);        							   
#endif
		createSignalChan(signalSock);			

		
		
		if ((passiveMode == true) && (multiFlows == 1)){
			
			GET_TIME_OF_DAY(&RifTime, _tend_, _tstart_, _sec_, _msec_, 1, 0);
			PRINTD(1,"identifySignalManager: Signaling Time: %lu.%06lu sec\n",
				(unsigned long int) RifTime.tv_sec, (unsigned long int) RifTime.tv_usec);

			
			okToNewSignalingChan=false;

			getnameinfo(DestHost->ai_addr,DestHost->ai_addrlen,globalSigAddr,INET6_ADDRSTRLEN,NULL,0,NI_NUMERICHOST);
			PRINTD(1,"identifySignalManager: global signaling address: %s, flowId=%d\n",globalSigAddr,flowId);
		}
		

		
		if ((namelogReceiver == 1) && (logServer == 0)) {
			if (sendNameLog(signalSock, logFileReceiver, sizeof(logFileReceiver)) < 0) {
				printf("sendNameLog() error \n");
				fflush(stdout);
				return (-1);
			}
			recv(signalSock, (char *) &type, sizeof(type), 0);
		}
		if (logServer == 1){
			if (sendLog(signalSock, serverLogReceiver, protoTx_ServerLogReceiver, logFileReceiver)<0){
				printf("sendLog() error \n");
				fflush(stdout);
				return(-1);
			}
			recv(signalSock, (char  *) &type, sizeof(type), 0);
		}

		
		
		*chanId = 0;
		while (signalChannels[*chanId].socket != -1)
			(*chanId)++;
		
		if (createNewPipe(signalChannels[*chanId].pipe) < 0) {
			printf("signalManager() could not open pipe for flowId %d\n", flowId);
			fflush(stdout);
			return(-1);
		}
		
		signalChannels[*chanId].DestAddr = DestHost;
		signalChannels[*chanId].flows = 1;
		signalChannels[*chanId].socket = signalSock;

		if ((logServer ==1 ) || (namelogReceiver ==1)){
			if (type == TSP_ERR_MSG_4){
				printf("signalManager() TSP_ERR_MSG_4\n");
				fflush(stdout);
				signalChannels[*chanId].errorLog = true;
			}
		}
		if (signalChannels[*chanId].errorLog == false){
			
			CREATE_THREAD(chanId, signalManager, NULL, signalChannels[*chanId].handle, true);

			
			signalChanCount++;
		}
		MUTEX_THREAD_UNLOCK(mutex);
		return(1);

	} else {
		
		
		if ((passiveMode == true) && (multiFlows == 1)){

			
			if (checkChanID == false){
				*chanId=0;
				signalChannels[*chanId].flows++;
			}
			PRINTD(1,"identifySignalManager: Using a old signaling channel,\n\tchanId=%d,\n\tflow_number=%d,\n\tglobal_signaling_address=%s\n\n",
					*chanId,signalChannels[*chanId].flows,globalSigAddr);

			
			if(DestHost) freeaddrinfo(DestHost);
			if(getaddrinfo(globalSigAddr, NULL, &hint, &(DestHost)) < 0){
				perror("identifySignalManager getaddrinfo() (1)");
				exitThread();
			}
			dstAddrSpecify = true;
		}
		

		MUTEX_THREAD_UNLOCK(mutex);
		return(0);
	}
}



void *flowSender(void *para)
{
	flowDescriptor *param = (flowDescriptor *) para;
	int id = param->id;

	
	struct addrinfo *SrcHost = flows[id].SrcHost;
	struct addrinfo *DestHost = flows[id].DestHost;
#if defined UNIX && ! defined BSD
	char* iface = flows[id].iface;
#endif
	BYTE meter = flows[id].meter;
	BYTE l4Proto = flows[id].l4Proto;
	BYTE l7Proto = flows[id].l7Proto;
	int icmptype = flows[id].icmptype;
	int DSByte = flows[id].DSByte;
	unsigned long Duration = flows[id].Duration;
	int TTL = flows[id].TTL;
	SumRandom * IntArriv = flows[id].IntArriv;
	SumRandom * PktSize = flows[id].PktSize;
	int sigChanId = flows[id].sigChanId;
	bool Nagle = flows[id].Nagle;
	struct info *infos = NULL;
	bool SrcPortSpecify = flows[id].srcPortSpecify;
	bool SrcAddrSpecify = flows[id].srcAddrSpecify;	
	unsigned int numPacket = flows[id].numPacket;	
	unsigned long int byteToSend = 1024 * flows[id].KBToSend;	

	char dotIpV4[INET_ADDRSTRLEN] = "0.0.0.0";	
	char dotIpV6[INET6_ADDRSTRLEN] = "::";		

	
	unsigned char payload[MAX_PAYLOAD_SIZE];

	unsigned char *ptrSeqNum = payload + sizeof(uint32_t);                  
	unsigned char *ptrTimeSec = ptrSeqNum + sizeof(uint32_t);               
	unsigned char *ptrTimeUsec = ptrTimeSec + sizeof(uint32_t);             
	unsigned char *ptrSize = NULL;		
	unsigned char *ptrToTruePayload = NULL;                                 

	long int indexBuffer = 0;                                               

	fd_set read_set, active_set;
	int exitflag, size, size_r = 0;
	long int time, time1, time2;
	struct TTicker Ticker;
	struct timeval start_time, end_time, RcvTime, timeout;

	int socktype;
	int prototype = 0;
	unsigned int seqNum = 1;
	unsigned int recvPkts = 0;
	Real wait;
	int count = 0;
	int sockchk = 0;
	struct pipeMsg msg;

	char HelpSrcAddress[INET6_ADDRSTRLEN];
	char HelpDstAddress[INET6_ADDRSTRLEN];
	int tmpPort_SrcPort = 0;
	int tmpPort_DstPort = 0;

	
	uint32_t net_id = 0;            
	uint32_t net_seqNum = 0;        
	uint32_t net_TimeSec = 0;       
	uint32_t net_TimeUsec = 0;      
	uint32_t net_size = 0;          

	
	
	unsigned int indexVectSize = 0;
	unsigned int dimVectSize = 0;
	unsigned int indexVectTime = 0;
	unsigned int dimVectTime = 0;
	bool SizeFile = false;
	bool TimeFile = false;
	

	unsigned long int byteSent = 0;	

	struct addrinfo *LogAddr;

#ifdef WIN32
	LARGE_INTEGER _tstart, _tend, _tprec;
	unsigned long secondi = 0, microsecondi = 0;
	int first_update = 1;
#endif

#ifdef SCTP
	int sctpId = flows[id].sctpId;
	int sctpStream;
	bool newSock = false;
	int ppid;
#endif

	
#ifdef BURSTY
	double OnDuration; 
	double OffDuration; 
	struct timeval StartOnTime, FinishOnTime, WaitTime;
	SumRandom * OnPeriod = 		flows[id].OnPeriod;
	SumRandom * OffPeriod = 	flows[id].OffPeriod;
#endif
	

	
	TSTART(_tstart, secondi, microsecondi, first, meter, 0);
	
	
	if (flows[id].Delay) {
		struct timeval ThisTime;

		PRINTD(1, "flowSender: Delay=%lu, flowId=%d\n", flows[id].Delay, id);

		
		flows[id].FlowStartTime.tv_sec = RifTime.tv_sec + flows[id].Delay / 1000 +
			(flows[id].FlowStartTime.tv_usec = RifTime.tv_usec + (flows[id].Delay % 1000) * 1000) / 1000000;

		flows[id].FlowStartTime.tv_usec %= 1000000;

		PRINTD(1,"flowSender: Expected Flow Start Time: %lu.%06lu sec, flowId=%d\n",
			(unsigned long int) flows[id].FlowStartTime.tv_sec,
			(unsigned long int) flows[id].FlowStartTime.tv_usec,id);

		GET_TIME_OF_DAY(&ThisTime, _tend_, _tstart_, _sec_, _msec_, meter, 0);
		while (timeval_subtract(&ThisTime,flows[id].FlowStartTime,ThisTime) == 0) {
			PRINTD(1,"flowSender: Timeout: %lu.%06lu sec, flowId=%d\n",
				(unsigned long int) ThisTime.tv_sec,
				(unsigned long int) ThisTime.tv_usec,id);

			ms_sleep(ThisTime);
			GET_TIME_OF_DAY(&ThisTime, _tend_, _tstart_, _sec_, _msec_, meter, 0);

			PRINTD(1,"flowSender: Real Flow Start Time: %lu.%06lu sec, flowId=%d\n",
				(unsigned long int) ThisTime.tv_sec,
				(unsigned long int) ThisTime.tv_usec,id);
		}
	}
	

	PRINTD(1,"flowSender: flowSender() n° %d started\n",id);

	
	if (logging) {
		infos = (struct info *) malloc(logbuffer_size * sizeof(info));
	}

	
	if (DestHost->ai_family == PF_INET6){
		getaddrinfo("::", NULL, &hint, &LogAddr);
	}else if (DestHost->ai_family == PF_INET){
		getaddrinfo("0.0.0.0", NULL, &hint, &LogAddr);
	}

	
	
	dimVectSize=flows[id].vectSize.size();
	if (dimVectSize > 0){
		SizeFile=true;
		indexVectSize=0;
		PRINTD(1,"flowSender: flows[id].vectSize.size() = %d\n",dimVectSize);
	}

	dimVectTime=flows[id].vectTime.size();
	if (dimVectTime > 0){
		TimeFile=true;
		indexVectTime=1;
		PRINTD(1,"flowSender: flows[id].vectTime.size() = %d\n",dimVectTime);
	}
	

	GET_TIME_OF_DAY(&start_time, _tend, _tstart, secondi, microsecondi, meter, 0);

	
	if (flows[id].dimPayloadBuffer == 0) {
		
		srand(start_time.tv_sec);
		for (int i = 0; i < MAX_PAYLOAD_SIZE; i++)
			payload[i] = (unsigned char) (rand() % 255);
	} else {
		
		for (int i = 0; i < MAX_PAYLOAD_SIZE; i++)
			payload[i] = 0;

		
		if (flows[id].payloadLogType == PL_STANDARD)
			ptrToTruePayload = ptrTimeUsec + sizeof(uint32_t);
		else if (flows[id].payloadLogType == PL_SHORT)
			ptrToTruePayload = ptrSeqNum + sizeof(uint32_t);
		else
			ptrToTruePayload = payload;
	}

	
	if (flows[id].payloadLogType != PL_NONE) {
		net_id = htonl(id); 
		
		memcpy(payload, &net_id, sizeof(net_id)); 
	}

	
	msg.flowId = id;

 	
 	if (SrcPortSpecify == true) {
 	    GET_PORT(flows[id].SrcHost, tmpPort_SrcPort);
 	}

	
 	if (SrcAddrSpecify == true){
 		if (SrcHost->ai_family == PF_INET6){
 			getnameinfo(SrcHost->ai_addr,SrcHost->ai_addrlen,dotIpV6,INET6_ADDRSTRLEN,NULL,0,NI_NUMERICHOST);
 		}else{
 			getnameinfo(SrcHost->ai_addr,SrcHost->ai_addrlen,dotIpV4,INET_ADDRSTRLEN,NULL,0,NI_NUMERICHOST);
 		}
 	}


	
	switch (l4Proto) {
	case L4_PROTO_UDP:
		socktype = SOCK_DGRAM;
		break;
	case L4_PROTO_ICMP:
		socktype = SOCK_RAW;
		if (meter == METER_OWDM)
			prototype = (DestHost->ai_family == AF_INET) ? IPPROTO_ICMP : IPPROTO_ICMPV6;
		else if (meter == METER_RTTM)
			prototype = IPPROTO_RAW;
		break;
#ifdef SCTP
	case L4_PROTO_SCTP:
		socktype = SOCK_STREAM;
		prototype = IPPROTO_SCTP;
		DestHost->ai_family = PF_INET;
		break;
#endif
#ifdef DCCP
	case L4_PROTO_DCCP:
		socktype = SOCK_DCCP;
		PRINTD(1,"flowSender: socktype DCCP  \n");
		break;
#endif
	default:
		socktype = SOCK_STREAM;
		break;
	}


#ifdef UNIX
	int sock;

#ifndef NOIPV6
	
	PRINTD(1,"flowSender: Try to create an IPv6 Socket\n");
	if (getaddrinfo(dotIpV6, NULL, &hint, &SrcHost) < 0)  {		
		
		PRINTD(1,"flowSender: Try to create an IPv4 Socket\n");
		if (SrcHost)
			freeaddrinfo(SrcHost);
#endif
		if (getaddrinfo(dotIpV4, NULL, &hint, &SrcHost) < 0) {		
			perror("flowSender");
			printf("Error into getaddrinfo Flow ID: %d\n", id);
			msg.code = MSG_SM_ERRFLOW;

			
			if (sendPipeMsg(signalChannels[sigChanId].pipe, &msg) < 0)
				perror("flowSender sending msg");
			exitThread();
		}
#ifndef NOIPV6
	}
#endif

#ifdef SCTP
	else if (l4Proto == L4_PROTO_SCTP) {
		DestHost->ai_family = PF_INET6;
		SrcHost->ai_family = PF_INET6;
	}
#endif
#endif

#ifdef WIN32
	unsigned int sock;
 #ifdef IPv6RECV
	if (getaddrinfo(dotIpV6, NULL, &hint, &SrcHost) <0){		
 #else
	if (getaddrinfo(dotIpV4, NULL, &hint, &SrcHost)<0){		
 #endif
		perror("flowSender");
		printf("Error into getaddrinfo Flow ID: %d\n", id);
		msg.code = MSG_SM_ERRFLOW;
		
		if (sendPipeMsg(signalChannels[sigChanId].pipe, &msg) < 0)
			perror("flowSender sending msg");
		exitThread();
	}
#endif

#ifdef SCTP
	MUTEX_THREAD_LOCK(mutexLog);
    if (l4Proto == L4_PROTO_SCTP && sctpSessions[sctpId].sock != -1) {
		
		sock = sctpSessions[sctpId].sock;
	} else {
    	
		sock = socket(DestHost->ai_family, socktype, prototype);
		sctpSessions[sctpId].sock = sock;
		newSock = true;
	}
	sctpStream = sctpSessions[sctpId].busyStreams++;
 	MUTEX_THREAD_UNLOCK(mutexLog);
#else
	
	sock = socket(DestHost->ai_family, socktype, prototype);
#endif
	if (sock < 0) {
		perror("flowSender");
		printf("Could not create a new socket. Flow ID: %d\n", id);
		msg.code = MSG_SM_ERRFLOW;
		
		if (sendPipeMsg(signalChannels[sigChanId].pipe, &msg) < 0)
			perror("flowSender sending msg");
		exitThread();
	}

#ifdef SCTP
	
	if (l4Proto == L4_PROTO_SCTP && newSock)
	{
		struct sctp_initmsg ginmsg;		

		ginmsg.sinit_num_ostreams = sctpSessions[sctpId].numStreams;
		ginmsg.sinit_max_instreams = sctpSessions[sctpId].numStreams;
		ginmsg.sinit_max_attempts = 0;
		ginmsg.sinit_max_init_timeo = 0;
		int initmsglen = sizeof(struct sctp_initmsg);
		if (setsockopt(sock, IPPROTO_SCTP, SCTP_INITMSG, &ginmsg, initmsglen) < 0){
			printf("flowSender sending msg\n");
			fflush(stdout);
		}
	}
#endif

#ifdef DCCP
	if (l4Proto == L4_PROTO_DCCP) {
		int optval = 0;
		if (setsockopt(sock, SOL_DCCP, DCCP_SOCKOPT_SERVICE, &optval, sizeof(optval))<0)
			printf("Error: flowSender setsockopt DCCP_SERVICE\n");
		PRINTD(1,"flowSender: setsockopt DCCP_SERVICE \n");
	}
#endif

	
#ifdef UNIX
	int reuse = 1;
	int optlen = sizeof(reuse);
 #ifdef SCTP
	if (l4Proto != L4_PROTO_SCTP)
 #endif
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, optlen)<0){
		printf("flowSender sending msg");
	}
#endif

	
	if (SrcPortSpecify || SrcAddrSpecify){

		
		
		if((passiveMode == true) && ((SrcHost->ai_family) != (DestHost->ai_family)) && (SrcHost->ai_family==PF_INET)){
			char oldAddr[INET_ADDRSTRLEN+1];
			char newAddr[INET6_ADDRSTRLEN+1];

			
			getnameinfo(SrcHost->ai_addr,SrcHost->ai_addrlen,oldAddr,INET_ADDRSTRLEN,NULL,0,NI_NUMERICHOST);
			
			freeaddrinfo(SrcHost);
			
			strcpy(newAddr,"::ffff:");
			strcat(newAddr,oldAddr);
			PRINTD(1,"flowSender: New IPv4 to IPv6 address %s\n",newAddr);
			
			getaddrinfo(newAddr, NULL, &hint, &SrcHost);
		}

#ifdef DEBUG
		char addr[INET6_ADDRSTRLEN+1];
		getnameinfo(SrcHost->ai_addr,SrcHost->ai_addrlen,addr,INET6_ADDRSTRLEN,NULL,0,NI_NUMERICHOST);
		PRINTD(1,"flowSender: Set Address/Port on Sender Side. \n\tAddress: %s \n\tPort number: %d \n", addr, ntohs(tmpPort_SrcPort));
#endif
		

		SET_PORT(SrcHost, tmpPort_SrcPort);
#ifdef SCTP
    	if (l4Proto != L4_PROTO_SCTP || (l4Proto == L4_PROTO_SCTP && newSock))
#endif
		if (bind(sock, SrcHost->ai_addr, SrcHost->ai_addrlen) != 0){
			perror("flowSender");
			printf("Could not bind a new socket. Flow ID: %d\n", id);
			msg.code = MSG_SM_ERRFLOW;
			
			if (sendPipeMsg(signalChannels[sigChanId].pipe, &msg) < 0)
				perror("flowSender sending msg");
			exitThread();
		}
	}


	
	if ((l7Proto != L7_PROTO_TELNET && l7Proto != L7_PROTO_DNS) && socktype == SOCK_STREAM && Nagle == false) {
		printf("Nagle algorithm disabled\n");
		fflush(stdout);
#ifdef UNIX
		int no_delay = 1;
		if (setsockopt(sock, getprotobyname("TCP")->p_proto, TCP_NODELAY, &no_delay, sizeof(no_delay)) < 0) {
#elif defined WIN32
		bool no_delayWin=true;																	

		if (setsockopt(sock, getprotobyname("TCP")->p_proto, TCP_NODELAY, (char *) no_delayWin, sizeof(no_delayWin)) < 0) {	
#endif
			printf("** WARNING ** Flow %d. Cannot disable Nagle Algorithm\n", id);
			fflush(stdout);
		}
	}

#if defined UNIX && ! defined BSD
	
	if (iface) {
		int dontRoute = 1;
		printf("Binding to device %s\n",iface);
		if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, iface, strlen(iface)) < 0) {
			printf("** WARNING ** Flow %d. Cannot bind to device %s (hint: you must be root)\n", id, iface);
			fflush(stdout);
		} else if (setsockopt(sock, SOL_SOCKET, SO_DONTROUTE, &dontRoute, sizeof(int)) < 0) {
			printf("** WARNING ** Flow %d. Cannot set don't route (hint: you must be root)\n", id);
			fflush(stdout);
		}
	}
#endif
	
 	MUTEX_THREAD_LOCK(mutexLog);
	if (logging && isFirstThread == 1) { 
		PRINTD(1,"flowSender: Log!  \n");
		isFirstThread = 0;
		isFirstThreadRttm = 0;
		if (!logremoto) {
			out.open(logFile, ios::out | ios::binary | ios::trunc);
			if (!out) {
				printf("** WARNING ** Flow %d. Cannot create local logfile\n", id);
				fflush(stdout);
				logging = 0;
			}
		}
		else
			createRemoteLogFile(logHost, protoTxLog);
	}
	MUTEX_THREAD_UNLOCK(mutexLog);

	
#ifdef SCTP
	if (l4Proto != L4_PROTO_SCTP || (l4Proto == L4_PROTO_SCTP && newSock)){
#endif

	if(passiveMode == false ){				

		if (connect(sock, DestHost->ai_addr, DestHost->ai_addrlen) < 0) {
			perror("flowSender");
			printf("Error in connect(). Flow ID: %d\n", id);
			
			msg.code = MSG_SM_ERRFLOW;
			if (sendPipeMsg(signalChannels[sigChanId].pipe, &msg) < 0)
				perror("flowSender sending msg");
			exitThread();
		}

	
	
	}else{
		if (flows[id].l4Proto == L4_PROTO_TCP){
			if (listen(sock, 1) < 0) {
				perror("flowSender");
				printf("Error in listen() (Passive Mode). Flow ID: %d\n", id);
				
				msg.code = MSG_SM_ERRFLOW;
				if (sendPipeMsg(signalChannels[sigChanId].pipe, &msg) < 0)
					perror("flowSender sending msg");
				exitThread();
			}
			PRINTD(1,"flowSender: socket in listening. (Passive Mode)\n");
		}
	}
	

#ifdef SCTP
	}
#endif

	
   	PRINTD(1,"flowSender: DS byte %d\n",DSByte);

	if (DSByte && (DestHost->ai_family == AF_INET) && setsockopt(sock,  SOL_IP,  IP_TOS, (char *) &DSByte, sizeof(DSByte)) < 0)
		printf("** WARNING ** Flow %d. Could not set DS byte to: %d\n", id, DSByte);

	if (DSByte && (DestHost->ai_family == AF_INET6))
#ifdef WIN32
		printf("** WARNING ** Flow %d. Windows does not allow to set the Traffic Class field in IPv6 headers\n",id);
#endif
#ifdef UNIX
#ifdef IPV6_TCLASS
		if (setsockopt(sock, SOL_IPV6, IPV6_TCLASS, (char *) &DSByte, sizeof(DSByte)) < 0)
			printf("** WARNING ** Flow %d. Could not set DS byte to: %d. Linux kernel >= 2.6.14 is required\n", id, DSByte);
#else
		printf("** WARNING ** Flow %d. Could not set DS byte to: %d. libc >= 2.4 is required\n", id, DSByte);
#endif
#endif

	
	PRINTD(1,"flowSender: TTL %d\n", TTL);
	if (TTL >0 && (setsockopt(sock, IPPROTO_IP, IP_TTL, (char *) &TTL, sizeof(TTL)) < 0))
		printf("** WARNING ** Flow %d. Could not set TTL to: %d\n", id, TTL);


#ifdef UNIX
	getsockname(sock, SrcHost->ai_addr, &SrcHost->ai_addrlen);
#endif
#ifdef WIN32
	getsockname(sock, SrcHost->ai_addr, (int *) &SrcHost->ai_addrlen);
#endif

   	
	getInfo(SrcHost, tmpPort_SrcPort, HelpSrcAddress);
	
	getInfo(DestHost, tmpPort_DstPort, HelpDstAddress);

#ifdef SCTP
	if (((l4Proto == L4_PROTO_TCP) || (l4Proto == L4_PROTO_SCTP)) && (flows[id].payloadLogType != PL_NONE)) {
#else
	if ((l4Proto == L4_PROTO_TCP) && (flows[id].payloadLogType != PL_NONE)) {
#endif
		if (flows[id].payloadLogType == PL_STANDARD)
			ptrSize = ptrTimeUsec + sizeof(uint32_t);
		else 
			ptrSize = ptrSeqNum + sizeof(uint32_t);



		
		ptrToTruePayload = ptrToTruePayload + sizeof(uint32_t);         	
	}

	
	
	
	
	if ((Duration == DefaultDuration) && (numPacket > 0)) {
		Duration = ULONG_MAX;
		PRINTD(1,"flowSender: Number of packets to generate : %d \n",numPacket);
	} else if (numPacket == 0) {
		numPacket = UINT_MAX;
	}
	

	
	
	if ((Duration == DefaultDuration) && (byteToSend > 0)){
		Duration=ULONG_MAX;
		PRINTD(1,"flowSender: Number of Kbyte to generate : %lu \n",flows[id].KBToSend);
	}else if(byteToSend == 0){
		byteToSend=ULONG_MAX;
	}
	


	
	if (passiveMode == true){
		msg.code = MSG_SM_SNDACK;
		
		if (sendPipeMsg(signalChannels[sigChanId].pipe, &msg) < 0){
			perror("flowSender sending msg");
			exitThread();
		}
		fflush(stdout);

		if ((flows[id].l4Proto == L4_PROTO_TCP)){
			PRINTD(1,"flowSender: Waiting for incoming connection. (Passive Mode)\n");

			int newSock=accept(sock,NULL,NULL);
			if (newSock < 0){
				perror("flowSender");
				printf("Error in accept() (Passive Mode). Flow ID: %d\n", id);
				
				msg.code = MSG_SM_ERRFLOW;
				if (sendPipeMsg(signalChannels[sigChanId].pipe, &msg) < 0)
					perror("flowSender sending msg");
				exitThread();
			}
			closeSock(sock);
			sock=newSock;

		}else if((flows[id].l4Proto == L4_PROTO_UDP)){
			char ch;

			PRINTD(1,"flowSender: Waiting \"Hole punching\" packets. (Passive Mode)\n");


#ifdef WIN32
			int len=flows[id].DestHost->ai_addrlen;
			recvfrom(sock,&ch,1,0,flows[id].DestHost->ai_addr, &len);
			flows[id].DestHost->ai_addrlen=len;
#else
			if(recvfrom(sock,&ch,1,0,flows[id].DestHost->ai_addr, &flows[id].DestHost->ai_addrlen)<0){
				PRINTD(1,"\nflowSender: Error:%s\n",strerror(errno));
				ReportErrorAndExit("flowSender", "error in recvfrom(). (Passive Mode)",programName, id);
			}
#endif

			if (connect(sock,flows[id].DestHost->ai_addr, flows[id].DestHost->ai_addrlen) < 0) {
				perror("flowSender");
				printf("Error in connect() (Passive Mode). Flow ID: %d\n", id);
				
				msg.code = MSG_SM_ERRFLOW;
				if (sendPipeMsg(signalChannels[sigChanId].pipe, &msg) < 0)
					perror("flowSender sending msg");
				exitThread();
			}

		}
	}
	

	printf("Started sending packets of flow ID: %d\n", id);
	fflush(stdout);

	Ticker.count = 0.0;
	
	

	
	TSTART(_tstart, secondi, microsecondi, first, meter, 0);
	GET_TIME_OF_DAY(&start_time, _tend, _tstart, secondi, microsecondi, meter, 0);

	
	end_time.tv_sec = start_time.tv_sec + Duration / 1000 + (end_time.tv_usec =
	    start_time.tv_usec + (Duration % 1000) * 1000) / 1000000;
	end_time.tv_usec %= 1000000;

	
	if (meter == METER_RTTM) {
		
		start_time.tv_sec = 0;
		start_time.tv_usec = 0;
		
		FD_ZERO(&active_set);
		FD_SET(sock, &active_set);
	}
#ifdef WIN32
	if (flows[id].serial != INVALID_HANDLE_VALUE){
		RTS_Enable (flows[id].serial);
		RTS_Disable(flows[id].serial);
		DTR_Enable (flows[id].serial);
	}
#endif

#ifdef UNIX
	if (flows[id].serial != INVALID_HANDLE_VALUE){
		RTS_Disable(flows[id].serial);
	}
#endif

	GET_TIME_OF_DAY(&Ticker.lastTime, _tend, _tstart, secondi, microsecondi, meter, 0);

	
	if(SizeFile==false){																
		
		size = (int) PktSize->Next();
	}else{																				
		
		size=flows[id].vectSize[indexVectSize];											
		
		indexVectSize = (indexVectSize + 1)%dimVectSize;								
	}
#ifdef SCTP
	if ((l4Proto == L4_PROTO_TCP || l4Proto == L4_PROTO_SCTP) && (flows[id].payloadLogType != PL_NONE)) {
#else
	if ((l4Proto == L4_PROTO_TCP) && (flows[id].payloadLogType != PL_NONE)) {
#endif
		flows[id].minPayloadSize = flows[id].minPayloadSize + sizeof(net_size); 

		if (size > MAX_PAYLOAD_SIZE)
			size = MAX_PAYLOAD_SIZE;
		else if (size < (flows[id].minPayloadSize)) 	
			size = flows[id].minPayloadSize;	
	} else {
		if (size > MAX_PAYLOAD_SIZE)
			size = MAX_PAYLOAD_SIZE;
		else if (size < flows[id].minPayloadSize)
			size = flows[id].minPayloadSize;
	}
	PRINTD(1,"flowSender: Payload minimum size for first packet %d \n",flows[id].minPayloadSize);
	PRINTD(1,"flowSender: Size for first packet %d\n",size);

 	
#ifdef BURSTY

	if (flows[id].bursty) {			
		
		OnDuration = (double) OnPeriod->Next();
		PRINTD(1,"flowSender: On Duration = %lf\n",OnDuration);
	} else {
		
		OnDuration = (double)Duration;
	}
#endif
	

	
	do {
		long int timeUsec;

		
#ifdef BURSTY
		do {
			
			gettimeofday(&StartOnTime, NULL);

			
			Ticker.count = 0.0;
#endif
			

			time = Ticker.lastTime.tv_sec % 86400L;
			timeUsec = Ticker.lastTime.tv_usec;

			
			net_seqNum = htonl(seqNum);
			net_TimeSec = htonl(time);
			net_TimeUsec = htonl(Ticker.lastTime.tv_usec);
			net_size = htonl(size);
			

			
			if (flows[id].payloadLogType == PL_STANDARD) { 
				memcpy(ptrSeqNum, &net_seqNum, sizeof(net_seqNum));             
				memcpy(ptrTimeSec, &net_TimeSec, sizeof(net_TimeSec));          
				memcpy(ptrTimeUsec, &net_TimeUsec, sizeof(net_TimeUsec));       
			} else if (flows[id].payloadLogType == PL_SHORT) 
				memcpy(ptrSeqNum, &net_seqNum, sizeof(net_seqNum));
#ifdef SCTP                                                                                                                            
			if (((l4Proto == L4_PROTO_TCP) || (l4Proto == L4_PROTO_SCTP)) && (flows[id].payloadLogType != PL_NONE))
#else                                                                  
			if ((l4Proto == L4_PROTO_TCP ) && (flows[id].payloadLogType != PL_NONE))
#endif
				memcpy(ptrSize, &net_size, sizeof(net_size));

			
			
			if (flows[id].dimPayloadBuffer != 0) {
				
				int byteToCopy = size - (int) (ptrToTruePayload - payload);

				
				if ((indexBuffer + byteToCopy) <= flows[id].dimPayloadBuffer) {
					
					memcpy(ptrToTruePayload, (void *) ((long int) flows[id].ptrPayloadBuffer + indexBuffer),
							byteToCopy);
					
					indexBuffer += byteToCopy;
					
					if (indexBuffer == flows[id].dimPayloadBuffer)
						
						indexBuffer = 0;
				} else {
					
					byteToCopy = flows[id].dimPayloadBuffer - indexBuffer;
					memcpy(ptrToTruePayload, (void *) ((long int) flows[id].ptrPayloadBuffer + indexBuffer),
							byteToCopy);
					
					memset((void *) ((long int) ptrToTruePayload + byteToCopy), 0, (size - byteToCopy));
					
					indexBuffer = 0;

				}
			}
			

			if ((l4Proto == L4_PROTO_ICMP) && (meter == METER_OWDM)) {
				if (DestHost->ai_family == AF_INET) {
					icmppkt pkt;
					pkt.icmp_buf.icmp_type = icmptype;
					pkt.icmp_buf.icmp_code = 0;
					pkt.icmp_buf.icmp_cksum = 0;
					memcpy(&pkt.packet, payload, size);
					pkt.icmp_buf.icmp_cksum = checksum((unsigned short *) &pkt, size + sizeof(icmp));
					sockchk = sendto(sock, (char *) &pkt, size + sizeof(icmp), 0, DestHost->ai_addr,
							DestHost->ai_addrlen);
				} else if (DestHost->ai_family == AF_INET6) {
#ifdef UNIX
					icmppktv6 pkt;
					pkt.icmp_buf.icmp_type = icmptype;
					pkt.icmp_buf.icmp_code = 0;
					pkt.icmp_buf.icmp_cksum = 0;
					memcpy(&pkt.packet, payload, size);
					pkt.icmp_buf.icmp_cksum = checksum((unsigned short *) &pkt, size + sizeof(icmpv6));
					struct sockaddr_in6 from;
					from.sin6_family = AF_INET6;
					from.sin6_port = 0;
					from.sin6_addr = ((struct sockaddr_in6*) (DestHost->ai_addr))->sin6_addr;
					sockchk = sendto(sock, (char *) &pkt, size + sizeof(icmpv6), 0, (sockaddr *) &from,
							sizeof(from));
#endif
				}
#ifndef SCTP
			} else {
				sockchk = send(sock, (char *) payload, size, 0);
			}
#else
			} else if (l4Proto != L4_PROTO_SCTP) {
				sockchk = send(sock, (char *) payload, size, 0);
			} else {
				ppid = rand();
				sockchk = sctp_sendmsg(sock, (char *) payload, size, (struct sockaddr *) DestHost->ai_addr,
						DestHost->ai_addrlen, ppid, 0, sctpStream, 0, 0);
			}
#endif

			if (flows[id].serial != INVALID_HANDLE_VALUE) {
				DTR_Disable(flows[id].serial);
				DTR_Enable(flows[id].serial);
			}

			if (sockchk < 0 && !termsock) {
#if DEBUG > 0
				
				printf("** WARNING ** flow: %d Packet: %d not sent\n", id, seqNum);
				perror("send()");
#endif
				fflush(stdout);
			} else {
				
				if ((logging) && (meter == METER_OWDM)) {
					if (seqNum == 1) {
						
#ifdef WIN32
						int len=LogAddr->ai_addrlen;
						getsockname(sock,LogAddr->ai_addr,&len);
						LogAddr->ai_addrlen=len;
#else
						getsockname(sock, LogAddr->ai_addr, &(LogAddr->ai_addrlen));
#endif
						getInfo(LogAddr, tmpPort_SrcPort, HelpSrcAddress);

#ifdef WIN32
						len=LogAddr->ai_addrlen;
						getsockname(sock,LogAddr->ai_addr,&len);
						LogAddr->ai_addrlen=len;
#else
						getpeername(sock, LogAddr->ai_addr, &(LogAddr->ai_addrlen));
#endif
						getInfo(LogAddr, tmpPort_DstPort, HelpDstAddress);
					}

					if (l7Proto == L7_PROTO_TELNET)
						size = size - 20;
					if (logremoto == 0) {
						
						writeInBufferStandard(&infos[count], htonl(id), htonl(seqNum), HelpSrcAddress,
								HelpDstAddress, tmpPort_SrcPort, tmpPort_DstPort, time, time,
								Ticker.lastTime.tv_usec, Ticker.lastTime.tv_usec, size); 
					} else {
						
						writeInBufferStandard(&infos[count], htonl(id), htonl(seqNum), HelpSrcAddress,
								HelpDstAddress, tmpPort_SrcPort, tmpPort_DstPort, time, time,
								Ticker.lastTime.tv_usec, Ticker.lastTime.tv_usec, size); 
						infosHostToNet(&infos[count]);
					}

					count++;
					
					if (count == logbuffer_size) {
						flushBuffer(infos, count);
						count = 0;
					}
				}
			}

			
			if (SizeFile == false) {									
				
				size = (int) PktSize->Next();
			} else {											
				
				size = flows[id].vectSize[indexVectSize];						
				
				indexVectSize = (indexVectSize + 1) % dimVectSize;					
			}
#ifdef SCTP
			if ((l4Proto == L4_PROTO_TCP || l4Proto == L4_PROTO_SCTP) && (flows[id].payloadLogType != PL_NONE)) {
#else
			if ((l4Proto == L4_PROTO_TCP) && (flows[id].payloadLogType != PL_NONE)) {
#endif
				if (size > MAX_PAYLOAD_SIZE)
					size = MAX_PAYLOAD_SIZE;
				else if (size < (flows[id].minPayloadSize))	
					size = flows[id].minPayloadSize;	
			} else {
				if (size > MAX_PAYLOAD_SIZE)
					size = MAX_PAYLOAD_SIZE;
				else if (size < flows[id].minPayloadSize)
					size = flows[id].minPayloadSize;
			}
			PRINTD(1,"flowSender: Payload minimum size for next packet %d \n",flows[id].minPayloadSize);
			PRINTD(2,"flowSender: Size for next packet %d\n",size);

			

			if (TimeFile == false) {
				wait = IntArriv->Next(); 
			} else {
				
				wait = flows[id].vectTime[indexVectTime];
				
				indexVectTime = (indexVectTime + 1) % dimVectTime;
			}

			PRINTD(2,"flowSender: IDT for next packet %f ms\n",wait);

			UPDATE_TICKER(&Ticker, _tend, _tprec, _tstart, first_update);
			if (flows[id].mean_adjustment == 0) 
				Ticker.count = 0;
			exitflag = 0;
			while (!exitflag) {
				if (wait <= Ticker.count) {
					exitflag = 1;
					timeout.tv_sec = 0;
					timeout.tv_usec = 0;
				} else {
					wait -= Ticker.count;
					Ticker.count = 0.0;

					
					if (flows[id].pollingMode == false) {						
						timeout.tv_sec = (long) wait / 1000;
						
						timeout.tv_usec = ((long) (wait * 1000) % 1000000);			
						
					} else {
						
						if (wait <= MAX_POLL) {
							
							timeout.tv_sec = 0;
							timeout.tv_usec = 0;
						} else {
							
							timeout.tv_sec = (long) (wait - MAX_POLL) / 1000;
							timeout.tv_usec = (long) ((wait - MAX_POLL) * 1000) % 1000000;
						}
					}
					
				}
				
				if (meter == METER_RTTM) {
					read_set = active_set;
					if (select(FD_SETSIZE, &read_set, NULL, NULL, &timeout) < 0) {
						perror("flowSender");
						printf("invalid fd or operation interrupted by signal\n");
						
						msg.code = MSG_SM_ERRFLOW;
						if (sendPipeMsg(signalChannels[sigChanId].pipe, &msg) < 0)
							perror("flowSender sending msg");
						exitThread();
					} 

					if (FD_ISSET(sock, &read_set)) {
						GET_TIME_OF_DAY(&RcvTime, _tend, _tstart, secondi, microsecondi, meter, 0);
						if (l4Proto == L4_PROTO_TCP)
							
							size_r = TCPrecvPacket((unsigned char *) payload, sock,
									flows[id].minPayloadSize, flows[id].payloadLogType);
#ifdef SCTP
						else if (l4Proto == L4_PROTO_SCTP) {
							size_r = SCTPrecvPacket((unsigned char *) payload, sock, sctpId,
									flows[id].minPayloadSize, flows[id].payloadLogType);
							PRINTD(2, "flowSender: in sctp settato flows[id].minPayloadSize: %d\n",
									flows[id].minPayloadSize);
						}
#endif
#ifdef DCCP					
						
						
#endif
						else
							size_r = recv(sock, (char *) payload, MAX_PAYLOAD_SIZE, 0);
						time1 = ntohl((*(uint32_t *) ptrTimeSec)); 
						time2 = RcvTime.tv_sec % 86400L;
						recvPkts++;

						
						if ((logging) && (size_r > 0) && (meter == METER_RTTM)) {
							if (logremoto == 0) {
								
								writeInBufferStandard(&infos[count], *(uint32_t *) payload,
										*(uint32_t *) ptrSeqNum, HelpSrcAddress,
										HelpDstAddress, tmpPort_SrcPort, tmpPort_DstPort,
										time1, time2, timeUsec, RcvTime.tv_usec, size_r);
							} else {
								
								writeInBufferStandard(&infos[count], *(uint32_t *) payload,
										*(uint32_t *) ptrSeqNum, HelpSrcAddress,
										HelpDstAddress, tmpPort_SrcPort, tmpPort_DstPort,
										time1, time2, timeUsec, RcvTime.tv_usec, size_r);
								infosHostToNet(&infos[count]);
							}
							count++;
							
							if (count == logbuffer_size) {
								flushBuffer(infos, count);
								count = 0;
							} 
						} 
					} 
				} 
				else {

					
					ms_sleep(timeout);
				}

				if (!exitflag)
					UPDATE_TICKER(&Ticker, _tend, _tprec, _tstart, first_update);
				else
					Ticker.count -= wait;
			} 

#ifdef WIN32
			GET_TIME_OF_DAY(&Ticker.lastTime, _tend, _tstart, secondi, microsecondi, meter, 0);
#endif
			seqNum++;

			
			byteSent += sockchk;
#if DEBUG > 2
			if (flows[id].KBToSend > 0)
			PRINTD(3,"Total bytes sent %lu = %f Kbyte \n",byteSent,((double)byteSent/1024));
#endif
			

			
#ifdef BURSTY

			
			gettimeofday(&FinishOnTime, NULL);
			
			OnDuration -= ((double) (FinishOnTime.tv_sec - StartOnTime.tv_sec) * 1000.0
					+ (double) (FinishOnTime.tv_usec - StartOnTime.tv_usec) / 1000.0);
			PRINTD(2,"flowSender: onDuration = %lf\n",OnDuration);

		} while ((OnDuration > 0)
				&& ((Ticker.lastTime.tv_sec < end_time.tv_sec)
				|| ((Ticker.lastTime.tv_sec == end_time.tv_sec)	&& (Ticker.lastTime.tv_usec < end_time.tv_usec)))
				&& ((seqNum - 1) < numPacket) && (byteSent < byteToSend));
		PRINTD(1,"flowSender: Exited from while with onDuration = %lf\n",OnDuration);

		if (flows[id].bursty) {
			
			OffDuration = (double) OffPeriod->Next();
			WaitTime.tv_sec = (long int) OffDuration / 1000;
			WaitTime.tv_usec = (OffDuration - (WaitTime.tv_sec * 1000)) * 1000;
			PRINTD(1,"flowSender: offDuration = %lf\n",OffDuration);
			
			ms_sleep(WaitTime);
			
			OnDuration = (double) OnPeriod->Next();
			PRINTD(1,"flowSender: On Duration = %lf\n",OnDuration);
		}
#endif
		

	} while ((Ticker.lastTime.tv_sec < end_time.tv_sec
			|| (Ticker.lastTime.tv_sec == end_time.tv_sec && Ticker.lastTime.tv_usec < end_time.tv_usec))
			&& ((seqNum - 1) < numPacket) && (byteSent < byteToSend));
	

	if (flows[id].serial != INVALID_HANDLE_VALUE)
		RTS_Enable(flows[id].serial);

	

	if (meter == METER_RTTM) {
		bool flag = true;
		while (flag && (seqNum-1) > recvPkts) {
			start_time.tv_sec = rttmDelay;
			read_set = active_set;
			if (select(FD_SETSIZE, &read_set, NULL, NULL, &start_time) < 0) {
				perror("send_packrttm");
				flag = true;
			} else {
				
				if (FD_ISSET(sock, &read_set)) {
					GET_TIME_OF_DAY(&RcvTime, _tend, _tstart, secondi, microsecondi, meter, 0);

					if (l4Proto == L4_PROTO_TCP)
						TCPrecvPacket((unsigned char *) payload, sock, flows[id].minPayloadSize,
								flows[id].payloadLogType);
#ifdef SCTP
					else if (l4Proto == L4_PROTO_SCTP) {
						SCTPrecvPacket((unsigned char *) payload, sock, sctpId, flows[id].minPayloadSize,
								flows[id].payloadLogType);
					}
#endif
#ifdef DCCP				
					
					
#endif
					else
						size_r = recv(sock, (char *) payload, MAX_PAYLOAD_SIZE, 0);

					time1 = ntohl((*(uint32_t *) ptrTimeSec)); 
					time2 = RcvTime.tv_sec % 86400L;
					int net_TimeUsec = ntohl(*(uint32_t *) ptrTimeUsec);
					recvPkts++;

					
					
					if (logging && size_r > 0) {
						if (logremoto == 0) {
							
							writeInBufferStandard(&infos[count], *(uint32_t *) payload,
									*(uint32_t *) ptrSeqNum, HelpSrcAddress, HelpDstAddress,
									tmpPort_SrcPort, tmpPort_DstPort, time1, time2, net_TimeUsec,
									RcvTime.tv_usec, size_r); 

						} else {
							
							writeInBufferStandard(&infos[count], *(uint32_t *) payload,
									*(uint32_t *) ptrSeqNum, HelpSrcAddress, HelpDstAddress,
									tmpPort_SrcPort, tmpPort_DstPort, time1, time2, net_TimeUsec,
									RcvTime.tv_usec, size_r); 
							infosHostToNet(&infos[count]);
						}
						count++;

						
						if (count == logbuffer_size) {
							flushBuffer(infos, count);
							count = 0;
						}
					} 
				} else {
					
					flag = false;
				} 
			} 
		} 
	}

	
	PRINTD(1,"flowSender: Finished sending packets of flow ID: %d\n", id);

	
	if ((logging) && (infos != NULL)) {
		flushBuffer(infos, count);
		count = 0;
	}

	if (infos != NULL && flows[id].l4Proto != L4_PROTO_SCTP)	
		free(infos);

#ifdef SCTP
	MUTEX_THREAD_LOCK(mutexSctp);
	sctpSessions[sctpId].busyStreams--;
	MUTEX_THREAD_UNLOCK(mutexSctp);
	if (l4Proto != L4_PROTO_SCTP || (l4Proto == L4_PROTO_SCTP && sctpSessions[sctpId].busyStreams == 0))
#endif
	closeSock(sock);

	
	if (!multiFlows)
		free(flows[id].ptrPayloadBuffer);

	
	flows[id].dimPayloadBuffer = 0;
	flows[id].ptrPayloadBuffer = NULL;


	
	msg.code = MSG_SM_ENDFLOW;
	if (sendPipeMsg(signalChannels[sigChanId].pipe, &msg) < 0) {
		perror("flow Sender sending msg");
		exitThread();
	}
	return NULL;
}


void Terminate(int sig)
{
	
	if (logremoto) {
		signaling signaling_log;
		signaling_log.stop = true;
		sendto(logSockSignaling, (char *) &signaling_log, sizeof(signaling_log), 0,
		    logHost->ai_addr, logHost->ai_addrlen);
		if (closeSock(logSock) < 0)
			printf("Error into close  log sock\n");
		if (closeSock(logSockSignaling) < 0)
			printf("Error into close log sock signaling\n");
	} else if (logging) {
		out.close();
	}

	
	
	
#ifdef UNIX
	memClean();
	exit(sig);
#endif
#ifdef WIN32
	WSACleanup();
	memClean();
#endif
}


#ifdef UNIX
#  define EXEC_NAME "      ./ITGSend"
#endif
#ifdef WIN32
#  define EXEC_NAME "    ITGSend.exe"
#endif
void printHelp()
{
	cout << "\nITGSend - Sender component of D-ITG platform\n";

	cout << "\n Working modes\n\n"

		"   ITGSend can work in three different modes:\n\n"

		"     - Single-flow: reads from the command line the traffic flow to generate\n"
		"     - Multi-flow:  reads from a script file the traffic flows to generate\n"
		"     - Daemon:      runs as a daemon to be remotely controlled using the ITGapi\n";

	cout << "\n Synopsis\n\n"

		"   * Single-flow mode *\n\n"

		"     "<<EXEC_NAME<<" [log_opts] [sig_opts] [flow_opts] [misc_opts]\n"
		"                     [ [idt_opts] [ps_opts] | [app_opts] ]\n\n"

		"   * Multi-flow mode *\n\n"

		"     "<<EXEC_NAME<<" <script_file> [log_opts]\n\n"

		"     where each line of the script file contains the single-flow mode options.\n\n"

		"   * Daemon mode *\n\n"

		"     "<<EXEC_NAME<<" -Q [log_opts]\n\n";

	cout << " Options\n";

	cout << "\n   Log options (log_opts):\n\n"

		"     -l [logfile]              Generate sender-side log file (default: " << DefaultLogFile << ").\n\n"

		"     -L [log_server_address]   Generate sender-side log file on a remote ITGLog instance\n"
		"        [logging_protocol]     (default: <" << DEFAULT_LOG_IP << "> <" << invFindL4Proto(DEFAULT_PROTOCOL_TX_LOG) << ">).\n\n"

		"     -x [receiver_logfile]     Ask ITGRecv to generate receiver-side log file (default: " << DefaultRecvLogFile << ").\n\n"

		"     -X [log_server_address]   Ask ITGRecv to generate receiver-side log file on a remote ITGLog instance\n"
		"        [logging_protocol]     (default: <" << DEFAULT_LOG_IP << "> <" << invFindL4Proto(DEFAULT_PROTOCOL_TX_LOG) << ">).\n\n"

		"     -q <log_buffer_size>      Number of packets to push to the log at once (default: " << logbuffer_size << ").\n\n";

	cout << "\n  Signaling options (sig_opts):\n\n"

		"     -Sda  <signaling_dest_addr>   Set the destination address for the signaling channel\n"
		"                                   (default: equal to -a <dest_address>).\n\n"

		"     -Sdp  <signaling_dest_port>   Set the destination port for the signaling channel\n"
		"                                   (default: " << DEFAULT_PORT_SIGNALING << ").\n\n"

		"     -Ssa  <signaling_src_addr>    Set the source address for the signaling channel\n"
		"                                   (default: Set by O.S.).\n\n"

		"     -Ssp  <signaling_src_port>    Set the source port for the signaling channel\n"
		"                                   (default: Set by O.S.).\n\n";
#ifdef UNIX
	cout <<	"     -Si  <signaling_interface>    Set the network interface for the signaling channel.\n\n";
#endif

	cout << "\n  Flow options (flow_opts):\n\n"

		"     -H                      Enable NAT traversal: FTP-like passive mode\n"
		"                             (please, refer to the manual for further details).\n\n"

		"     -m  <meter>             Set the type of meter (default: owdm):\n"
		"                             - owdm (one-way delay meter)\n"
		"                             - rttm (round-trip time meter)\n\n"

		"     -t  <duration>          Set the generation duration in ms (default: " << DefaultDuration << " ms).\n\n"

		"     -z  <#_of_packets>      Set the number of packets to generate\n\n"

		"     -k  <#_of_KBytes>       Set the number of KBytes to generate\n\n"

		"     -d  <delay>             Start the generation after the specified delay in ms (default: " << DefaultDelay << " ms).\n\n"

		"     -b <DS_byte>            Set the DS byte for QoS tests (default: " << DefaultDSByte << ").\n\n"

		"     -f <TTL byte>           Set the IP Time To Live (default:  64).\n\n"

		"     -a  <dest_address>      Set the destination address (default: " << DefaultDestIP << ").\n\n"

		"     -sa <src_address>       Set the source address (default: Set by O.S.).\n\n"

		"     -rp <dest_port>         Set the destination port (default: " << DEFAULT_PORT << ").\n\n"

		"     -sp <src_port>          Set the source port (default: Set by O.S.).\n\n";
#ifdef UNIX
	cout << "     -i <interface>          Bind to the given interface (default: don't bind to any interface).\n\n";
#endif
	cout << "     -p <payload_metadata>   Select the metadata sent in the payload of each packet (default: 2).\n"
		"                             (please, refer to the manual for further details).\n\n"

		"     -T <protocol>           Layer 4 protocol (default: " << invFindL4Proto(L4_PROTO_UDP) << "):\n"
		"                             - UDP                   (User Datagram Protocol)\n"
		"                             - TCP                   (Transport Control Protocol)\n"
		"                             - ICMP [type]           (Internet Control Messaging Protocol)\n";
#ifdef SCTP
	cout << "                             - SCTP <association_id> (Session Control Transport Protocol)\n"
		"                                    <max_streams>\n";
#endif
#ifdef DCCP
	cout << "                             - DCCP                  (Datagram Congestion Control Protocol)\n";
#endif
	cout << "\n     -D                      Disable TCP Nagle algorithm.\n\n";


	cout << "\n  Inter-departure time options (idt_opts):\n\n"

		"     -C  <rate>                 Constant (default: " << DefaultPktPerSec << " pkts/s).\n\n"

		"     -U  <min_rate>             Uniform distribution.\n"
		"         <max_rate>\n\n"

		"     -E  <mean_rate>            Exponential distribution.\n\n"

		"     -N  <mean> <std_dev>       Normal distribution.\n\n"

		"     -O  <mean>                 Poisson distribution.\n\n"

		"     -V  <shape> <scale>        Pareto distribution.\n\n"

		"     -Y  <shape> <scale>        Cauchy distribution.\n\n"

		"     -G  <shape> <scale>        Gamma distribution.\n\n"

		"     -W  <shape> <scale>        Weibull distribution.\n\n"

		"     -Ft <filename>             Read IDTs from file.\n\n";
#ifdef BURSTY
	cout << "     -B  <onDistro> <params>    Generate bursty traffic:\n"
		"         <offDistro> <params>   - set the duration of both ON and OFF periods according to a\n"
		"                                  supported random distribution (e.g. -B C 1000 C 1000).\n\n";
#endif

	cout << "\n  Packet size options (ps_opts):\n\n"

		"     -c  <pkt_size>           Constant (default: " << DefaultPktSize << " bytes).\n\n"

		"     -u  <min_pkt_size>       Uniform distribution.\n"
		"         <max_pkt_size>\n\n"

		"     -e  <average_pkt_size>   Exponential distribution.\n\n"

		"     -n  <mean> <std_dev>     Normal distribution.\n\n"

		"     -o  <mean>               Poisson distribution.\n\n"

		"     -v  <shape> <scale>      Pareto distribution.\n\n"

		"     -y  <shape> <scale>      Cauchy distribution.\n\n"

		"     -g  <shape> <scale>      Gamma distribution.\n\n"

		"     -w  <shape> <scale>      Weibull distribution.\n\n"

		"     -Fs <filename>           Read payload sizes from file.\n\n";

	cout << "\n Application layer options (app_opts): \n\n"

		"     -Fp <filename>        Read payload content from file.\n\n"

		"     Telnet                Emulate¹ Telnet traffic.\n\n"

		"     DNS                   Emulate¹ DNS traffic.\n\n"

		"     Quake3                Emulate¹ Quake 3 traffic.\n\n"

		"     CSa                   Emulate¹ Counterstrike traffic - active player.\n\n"

		"     CSi                   Emulate¹ Counterstrike traffic - idle player.\n\n"

		"     VoIP                  Emulate¹ Voice-over-IP traffic.\n"
		"          -x <codec>       VoIP sub-option: audio codec (default: G.711.1):\n"
		"                             - G.711.<1 or 2> (samples per pkt)\n"
		"                             - G.729.<2 or 3> (samples per pkt)\n"
		"                             - G.723.1\n"
		"          -h <protocol>    VoIP sub-option: audio transfer protocol (default: RTP).\n"
		"                             - RTP:  Real Time Protocol (default)\n"
		"                             - CRTP: Real Time Protocol with header compression\n"
		"          -VAD             VoIP sub-option: enable voice activity detection\n\n"

		"   ¹ Emulation is obtained by properly replicating packet sizes and IDTs.\n\n";

	cout << "\n  Misc options (misc_opts):\n\n"

		"     -h | --help          Display this help and exit.\n\n"

		"     -s  <seed>           Set the seed used for generating distributions (default: random).\n\n"

		"     -poll                Use busy-wait loop for IDTs shorter than 1 msec.\n\n"

		"     -j  <0|1>            Guarantee the mean packet rate (default: " << mean_adjustment << "):\n"
		"                          - 0 (disable)\n"
		"                          - 1 (enable)\n\n"

		"     -sk <serial_iface>   Raise a signal on the serial interface when sending packets.\n\n"

		"     -rk <serial_iface>   Ask ITGRecv to raise a signal on the serial interface when receiving packets.\n\n";
#ifdef WIN32
	cout << "     -P                   Enable thread high priority.\n\n";
#endif

	cout << "\nFor more information please refer to the manual.\n";

	exit(1);
}

void ReportErrorAndExit(const char *function, const char *msg, char *pname, int fid)
{
	fprintf(stderr, " ** ERROR **  Flow %d aborted:  %s : %s \n ", fid, function, msg);
	fprintf(stderr, "Type %s -h for help \n", pname);
	fflush(stderr);
	memClean();
	exit(1);
}


#ifdef UNIX

void createTransportChan(int &signalSock, struct addrinfo* &DestHost,bool &dstAddrSpecify,struct addrinfo* &SrcHost,uint16_t SrcPort,const char * Interface)
#else

void createTransportChan(unsigned int &signalSock, struct addrinfo* &DestHost,bool &dstAddrSpecify,struct addrinfo* &SrcHost,uint16_t SrcPort)	
#endif
{
	struct addrinfo * TmpAddress = NULL;	
	int reuse = 1;				
	int newSock = 0;			
	uint16_t port = 0;			

	
	
#if defined UNIX && ! defined BSD
	if (Interface!=NULL){
		printf("\nBinding to device %s for signaling channel\n",Interface);
		if (setsockopt(signalSock, SOL_SOCKET, SO_BINDTODEVICE, Interface, strlen(Interface)) < 0) {
			perror("Cannot bind to device (hint: you must be root)");
			memClean();
			exit(1);
		}
	}
#endif

	if (passiveMode==false){	

		
		signalSock = socket(DestHost->ai_family, SOCK_STREAM, 0);

		
		if(SrcHost!=NULL){
			SET_PORT(SrcHost,htons(SrcPort));
			
		}else if (SrcPort != 0){
#if (defined WIN32 && defined IPv6RECV) || defined UNIX || defined BSD
			if (DestHost->ai_family == PF_INET6){
				if (SrcHost) freeaddrinfo(SrcHost);
				getaddrinfo("::", NULL, &hint, &SrcHost);
			}else
#endif
			{
				if (SrcHost) freeaddrinfo(SrcHost);
				getaddrinfo("0.0.0.0", NULL, &hint, &SrcHost);
			}
			SET_PORT(SrcHost,htons(SrcPort));
		}

		
		if((SrcHost!=NULL) || (SrcPort != 0)){
			if (bind(signalSock, SrcHost->ai_addr, SrcHost->ai_addrlen) != 0){
				perror("Bind error in createTransportChan()");
				memClean();
				exit(1);
			}
		}
		

#ifdef UNIX
    	
    	if (Interface!=NULL){
    		int dontRoute = 1;
    		if (setsockopt(signalSock, SOL_SOCKET, SO_DONTROUTE, &dontRoute, sizeof(int)) < 0) {
    			perror("Cannot set don't route (hint: you must be root)");
    			memClean();
    			exit(1);
    		}
    	}
#endif

    	
    	if ( connect(signalSock, DestHost->ai_addr, DestHost->ai_addrlen) < 0) {
			perror("Connect error in createTransportChan()");
			memClean();
			exit(1);
		}
		
	}else{
		
		if(SrcHost==NULL) {
#if (defined WIN32 && defined IPv6RECV) || defined UNIX || defined BSD
			
			if (SrcHost) freeaddrinfo(SrcHost);
			if(getaddrinfo("::", NULL, &hint,&SrcHost) < 0)
#endif
			{
				
				if(SrcHost) freeaddrinfo(SrcHost);
				if(getaddrinfo("0.0.0.0", NULL, &hint, &SrcHost) < 0){
					perror("getaddrinfo error in createTransportChan() (Passive Mode)");
					memClean();
					exit(1);
				}
			}
		}

		
#if (defined WIN32 && defined IPv6RECV) || defined UNIX || defined BSD
		if (SrcHost->ai_family == PF_INET6){
			 if (TmpAddress) freeaddrinfo(TmpAddress);
			getaddrinfo("::", NULL, &hint,&TmpAddress);
			PRINTD(1,"createTransportChan: bind to IPv6 socket\n");
		} else
#endif
		{
			if (TmpAddress) freeaddrinfo(TmpAddress);
			getaddrinfo("0.0.0.0", NULL, &hint, &TmpAddress);
			PRINTD(1,"createTransportChan: bind to IPv4 socket\n");
		}

		
		if (SrcPort != 0){
			SET_PORT(SrcHost,htons(SrcPort));
		}else{
			SET_PORT(SrcHost,htons(DEFAULT_PORT_SIGNALING));
		}

		
		signalSock = socket(SrcHost->ai_family, SOCK_STREAM, 0);

		if (setsockopt(signalSock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse))<0){
			perror("Setsockopt in createTransportChan() (Passive Mode)");
			memClean();
			exit(1);
		}

		
		if (bind(signalSock, SrcHost->ai_addr, SrcHost->ai_addrlen) != 0){
			perror("Bind error in createTransportChan() (Passive Mode)");
			memClean();
			exit(1);
		}

		
		if (listen(signalSock, SOMAXCONN) == 0){
			PRINTD(1,"createTransportChan: Listening\n");
		}else{
			perror("Listen in createTransportChan() (Passive Mode)");
			memClean();
			exit(1);
		}

		
		if ((newSock = accept(signalSock,TmpAddress->ai_addr, (socklen_t *) &(TmpAddress->ai_addrlen))) < 0){
			perror("Accept in createTransportChan() (Passive Mode)");
			memClean();
			exit(1);
		}
		PRINTD(1,"createTransportChan: New signaling connection accepted (Passive mode)\n");
		
		closeSock(signalSock);
		
		signalSock=newSock;

		
		if (dstAddrSpecify == false){

			char *buffer=NULL;

			
			if (TmpAddress->ai_family==PF_INET){
				buffer=new char[INET_ADDRSTRLEN];
				
				getnameinfo(TmpAddress->ai_addr,TmpAddress->ai_addrlen,buffer,INET_ADDRSTRLEN,NULL,0,NI_NUMERICHOST);
			}else if (TmpAddress->ai_family==PF_INET6){
				
				buffer=new char[INET6_ADDRSTRLEN];
				
				getnameinfo(TmpAddress->ai_addr,TmpAddress->ai_addrlen,buffer,INET6_ADDRSTRLEN,NULL,0,NI_NUMERICHOST);
			}
			PRINTD(1,"createTransportChan: take the address of the receiver: %s\n",buffer);

			
			GET_PORT(DestHost,port);
			
			if (DestHost) freeaddrinfo(DestHost);
			getaddrinfo(buffer, NULL, &hint, &DestHost);
			SET_PORT(DestHost,port);
			
			dstAddrSpecify = true;

			delete buffer;
			if (TmpAddress) freeaddrinfo(TmpAddress);
		}
	}
	
}


void sendType(int signalSock, BYTE type)
{
	int err;
	char msg[1];

	putValue(&msg, (void *) &type, sizeof(type));

#ifdef UNIX
        err = send(signalSock, (char *) &msg, sizeof(msg), MSG_DONTWAIT);
      	if (err == EAGAIN) PRINTD(1,"sendType: Send type would block! **\n");
#endif

#ifdef WIN32
      	int timeo= 100000;
      	if (setsockopt(signalSock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeo,sizeof(timeo)) < 0)
          	printf("Error on setting timeout for sending 1\n");
      	err = send(signalSock, (char *) &msg, sizeof(msg), 0);
      	if(err<0)
         	PRINTD(1,"sendType: Send type would block!\n");
      	timeo=0;
      	if (setsockopt(signalSock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeo,sizeof(timeo)) < 0)
          	printf("Error on setting timeout for sending 2\n");
#endif

}


#ifdef UNIX
void createSignalChan(int signalSock)
#else
void createSignalChan(unsigned int signalSock)
#endif
{
	timeval timeout;
	int numSend = 0;
	fd_set active_set;
	int exit = 0;
	do {
		
		sendType(signalSock, TSP_CONNECT);
		PRINTD(1,"createSignalChan: sent TSP_CONNECT\n");

		
		FD_ZERO(&active_set);
		FD_SET(signalSock, &active_set);
		timeout.tv_sec = TIME_OUT;
		timeout.tv_usec = 0;
		if (select(FD_SETSIZE, &active_set, NULL, NULL, &timeout) < 0) {
			printf("Error ** : Invalid file descriptor\n");
		}
		if (FD_ISSET(signalSock, &active_set)) {
			recvAck(signalSock);
			exit = 1;
		}
		numSend++;
	} while ((exit != 1) && (numSend < 2));

	if (numSend == 2){
		printf("** ERROR ** Receiver is down !\n");
		fflush(stdout);
	}
}


#ifdef UNIX
void sendRelease(int signalSock)
#else
void sendRelease(unsigned int signalSock)
#endif
{
	timeval timeout;
	int numSend = 0;
	fd_set active_set;
	int exit = 0;
	do {
		sendType(signalSock, TSP_RELEASE);
		PRINTD(1,"sendRelease: sent TSP_RELEASE\n");
		FD_ZERO(&active_set);
		FD_SET(signalSock, &active_set);
		timeout.tv_sec = TIME_OUT;
		timeout.tv_usec = 0;
		if (select(FD_SETSIZE, &active_set, NULL, NULL, &timeout) < 0) {
			printf("Error ** : Invalid file descriptor\n");
		}
		if (FD_ISSET(signalSock, &active_set)) {
			recvAck(signalSock);
			PRINTD(1,"sendRelease: received ack to release signal channel\n");
			exit = 1;
		}
		numSend++;
	} while ((exit != 1) && (numSend < 2));
	if (numSend == 2) {
		printf("** ERROR ** Receiver is down !\n");
		fflush(stdout);
	}
}


int sendNameLog(int signalSock, char *FileName, int sizeFileName)
{
	char Messaggio[DIM_LOG_FILE + sizeof(BYTE)];
	char *next;

	PRINTD(1,"sendNameLog: sent TSP_SEND_NAME_LOG\n");
	BYTE type = TSP_SEND_NAME_LOG;
	int ret = 0;
	int sizeMessag;
	next = putValue(&Messaggio, (void *) &type, sizeof(type));
	next = putValue(next, (void *) FileName, sizeFileName);
	sizeMessag = sizeof(type) + sizeFileName;
	ret  = send(signalSock, (char *) &Messaggio, sizeMessag, 0);
	return (ret);
}




int sendLog(int signalSock, struct addrinfo *logHost, BYTE logProtocol, char *FileName)
{
	char Messaggio[DIM_LOG_FILE + 20];
	char *next;
	PRINTD(1,"sendLog: sent TSP_SEND_FLOW_LOG\n");
	BYTE type = TSP_SEND_FLOW_LOG;
	int ret, sizeMessag;
	int net_ai_family = htonl(logHost->ai_family); 
	next = putValue(&Messaggio, (void *) &type, sizeof(type));
	next = putValue(next, &net_ai_family, sizeof(int));
	if (logHost->ai_family == PF_INET) {
		next =
		    putValue(next,
		    (void *) &(((struct sockaddr_in *) logHost->ai_addr)->sin_addr.s_addr),
		    sizeof(in_addr_t));
		next =
		    putValue(next, (void *) &(((struct sockaddr_in *) logHost->ai_addr)->sin_port),
		    sizeof(in_port_t));
	} else if (logHost->ai_family == PF_INET6) {
		next =
		    putValue(next,
		    (void *) ( &((struct sockaddr_in6 *) logHost->ai_addr)->sin6_addr), 
		    4 * sizeof(in_addr_t));
		next =
		    putValue(next,
		    (void *) &((
		    (struct sockaddr_in6 *) logHost->ai_addr)->sin6_port),
		    sizeof(in_port_t));
		
	}
	next = putValue(next, (void *) &logProtocol, sizeof(logProtocol));
	next = putValue(next, (void *) FileName, DIM_LOG_FILE);

	sizeMessag = sizeof(type) + sizeof(int) + sizeof(in_port_t) + sizeof(BYTE) + DIM_LOG_FILE;
	if (logHost->ai_family == PF_INET)
		sizeMessag += sizeof(in_addr_t);
	else if (logHost->ai_family == PF_INET6)
		sizeMessag += 4 * sizeof(in_addr_t);

	ret = send(signalSock, (char *) &Messaggio, sizeMessag, 0);
	return (ret);
}


void recvAck(int signalSock)
{
	int i;
	char buffer;
	i = recv(signalSock, (char *) &buffer, sizeof(BYTE), 0);
    if (i < 0) {
        perror("recvAck: error while receiving ACK on the signaling channel");
    }
}


void closedFlowErr(int flowId, int signalSock)
{
	char *next;
	PRINTD(1,"closedFlowErr: sent TSP_CLOSED_ERR\n");
	BYTE type = TSP_CLOSED_ERR;
	int sizeMessag = sizeof(int) + sizeof(type);
	char Messaggio[sizeMessag];
	int net_flowId = htonl(flowId); 

	next = putValue(&Messaggio, (void *) &type, sizeof(type));
	next = putValue(next, (void *) &net_flowId, sizeof(int));
	int size;
	size = send(signalSock, (char *) &Messaggio, sizeMessag, 0);
	if (size < 0) {
		printf("error into send\n");
		memClean();
		exit(1);
	}
}


int closedFlow(int flowId, int signalSock)
{
	char *next;
	PRINTD(1,"closedFlow: sent TSP_CLOSED_FLOW\n");
	BYTE type = TSP_CLOSED_FLOW;
	int sizeMessag = sizeof(int) + sizeof(type);
	char Messaggio[sizeMessag];
	int net_flowId = htonl(flowId); 

	next = putValue(&Messaggio, (void *) &type, sizeof(type));
	next = putValue(next, (void *) &net_flowId, sizeof(int));

	if (send(signalSock, (char *) &Messaggio, sizeMessag, 0) < 0) {
		printf("Error in request to close\n");
		fflush(stdout);
		return(-1);
	}

	return (0);
}



int sendAckFlow(int flowId, int signalSock)
{
	char *next;
	PRINTD(1,"sendAckFlow: sent TSP_ACK_SEND_FLOW\n");
	BYTE type = TSP_ACK_SEND_FLOW;
	int sizeMessag = sizeof(int) + sizeof(type);
	char Messaggio[sizeMessag];
	int net_flowId = htonl(flowId); 

	next = putValue(&Messaggio, (void *) &type, sizeof(type));
	next = putValue(next, (void *) &net_flowId, sizeof(int));

	if (send(signalSock, (char *) &Messaggio, sizeMessag, 0) < 0) {
		printf("Error in send ack flow\n");
		fflush(stdout);
		return(-1);
	}

	return (0);
}



int requestToSend(int flowId, int signalSock)
{
	char Messaggio[100];
	char *next;
	BYTE type = TSP_SEND_FLOW;				
	int sizeMessag = 0;
	uint16_t srcPort = 0; 					
	uint16_t netSrcPort = 0;
	uint16_t netDstPort = 0;

	struct addrinfo *DestHost;				
	if (passiveMode == false)				
		DestHost = flows[flowId].DestHost;		
	else{							
		DestHost = flows[flowId].SrcHost;		
		GET_PORT((flows[flowId].DestHost),srcPort);	
		netSrcPort = htons(srcPort);
	}

	BYTE l4Proto = flows[flowId].l4Proto;
	BYTE l7Proto = flows[flowId].l7Proto;
	BYTE meter = flows[flowId].meter;
	BYTE dsByte = flows[flowId].DSByte;
	BYTE l3Proto = (DestHost->ai_family == PF_INET) ? L3_PROTO_IPv4 : L3_PROTO_IPv6;
	BYTE srcAddrSpecify = (flows[flowId].srcAddrSpecify == true) ? 0xFF : 0x00;
	BYTE plType = flows[flowId].payloadLogType;

	
	uint32_t net_flowId = htonl(flowId);

	PRINTD(1,"requestToSend: sent TSP_SEND_FLOW\n");

	
	next = putValue(&Messaggio, (void *) &type, sizeof(BYTE));
	next = putValue(next, (void *) &l4Proto, sizeof(BYTE));
	next = putValue(next, (void *) &l7Proto, sizeof(BYTE));
	next = putValue(next, (void *) &meter, sizeof(BYTE));
	next = putValue(next, (void *) &dsByte, sizeof(BYTE));
	next = putValue(next, (void *) &l3Proto, sizeof(BYTE));
	next = putValue(next, (void *) &net_flowId, sizeof(uint32_t));
	next = putValue(next, (void *) &plType, sizeof(BYTE));
 	next = putValue(next, (void *) &flows[flowId].serialReceiver, DIM_NAME_SERIAL_INTERFACE);
	sizeMessag += 7 * sizeof(BYTE) + sizeof(uint32_t) + DIM_NAME_SERIAL_INTERFACE;

	
 	
 	
 	
 	if (passiveMode == true){							
 		next = putValue(next, (void *) &(netSrcPort), sizeof(uint16_t));	
 		sizeMessag += sizeof(uint16_t);						

 		next = putValue(next, (void *) &(srcAddrSpecify), sizeof(BYTE));	
 		sizeMessag += sizeof(BYTE);						
 	}

	if (DestHost->ai_family == PF_INET) {
		in_addr_t DstAddr = ((struct sockaddr_in *) DestHost->ai_addr)->sin_addr.s_addr;
		netDstPort = htons(((struct sockaddr_in *) DestHost->ai_addr)->sin_port);

		

		next = putValue(next, (void *) &netDstPort, sizeof(in_port_t));
		
		next = putValue(next, (void *) &DstAddr, sizeof(in_addr_t));

		sizeMessag += sizeof(in_port_t) + sizeof(in_addr_t);
	} else if (DestHost->ai_family == PF_INET6) {
		struct in6_addr *DstAddr = &((struct sockaddr_in6 *) DestHost->ai_addr)->sin6_addr;
		netDstPort = htons(((struct sockaddr_in6 *) DestHost->ai_addr)->sin6_port);
		

		
		next = putValue(next, (void *) &netDstPort, sizeof(in_port_t));
		
		next = putValue(next, (void *) DstAddr, 4 * sizeof(in_addr_t));
		
		
		

		sizeMessag += sizeof(in_port_t) + 4 * sizeof(in_addr_t); 
	}

	PRINTD(1,"requestToSend: new flow message prepared \n");

	if (send(signalSock, (char *) &Messaggio, sizeMessag, 0) < 0) {
		printf("Error in request to send\n");
		fflush(stdout);
		return(-1);
	}
	PRINTD(1,"requestToSend: new flow message sent \n");
	return(0);
}


bool checkDestHostIP(int * chanId, struct addrinfo * DestHost)
{
	int i = 0;

	while (i < MAX_NUM_THREAD) {
		

		if ((signalChannels[i].socket != -1)
		  && (ARE_INET_ADDR_EQUAL(DestHost, signalChannels[i].DestAddr))) {
		      signalChannels[i].flows++;
		      *chanId = i;
		      return true;
        }
		i++;
	}
	return false;
}

void argvToString(char *argv[], int argc, char line[200])
{
	int i = 0;

	strcpy(line, "^");
	while (argc > 0) {
		strcat(line, argv[i]);
		strcat(line, "^");
		argc--;
		i++;
	}
	strcat(line, "\n");
}


void flushBuffer(info *infos, int count)
{
	if (logremoto) {
		MUTEX_THREAD_LOCK(mutexLogRem);
		if(sendto(logSock, (char *) infos, count * sizeof(struct info), 0, logHost->ai_addr,
		    logHost->ai_addrlen) < 0) {
			printf("ERROR Sending infos to logger....\n");
			fflush(stdout);
		}
		MUTEX_THREAD_UNLOCK(mutexLogRem);
	} else {
		MUTEX_THREAD_LOCK(mutexLog);
		out.write((const char *) infos, count * sizeof(struct info));
		out.flush();
		MUTEX_THREAD_UNLOCK(mutexLog);
	}
}


void memClean()
{
#ifdef WIN32
 	
 	ExitProcess(0);
#endif
}




void createRemoteLogFile(struct addrinfo *logHost, BYTE protocolLog)
{
	
	
	
	

	uint32_t port;
	

	createSignalingLogChannel(protocolLog);

	

	switch(protocolLog) {
		case L4_PROTO_UDP :
		
			logSock = socket(logHost->ai_family, SOCK_DGRAM, 0);
			if (logSock < 0)
				ReportErrorAndExit("createRemoteLogFile","socket DATAGRAM - Cannot create socket logSock", programName, 0);
			break;
		case L4_PROTO_TCP :
            
			logSock = socket(logHost->ai_family, SOCK_STREAM, 0);
			if (logSock < 0)
				ReportErrorAndExit("createRemoteLogFile","socket STREAM - Cannot create socket logSock", programName, 0);
			break;
		default :
			break;
	}
	
	if ( recv(logSockSignaling, (char *) &port, sizeof(port), 0) < 0)
		ReportErrorAndExit("createRemoteLogFile","Cannot receive on signaling socket", programName, 0);
	port = ntohl(port);
	PRINTD(1,"createRemoteLogFile: Port received for log : %d\n",port);


	
	SET_PORT(logHost, htons(port));
	sleep(1);
    
	if (connect(logSock, logHost->ai_addr, logHost->ai_addrlen) < 0)
		ReportErrorAndExit("createRemoteLogFile","Error into connect with logsender!", programName, 0);
}



void createSignalingLogChannel(BYTE protocolLog)
{
	
	signaling signal;
	
	char buffer[1];

	

	logSockSignaling = socket(logHost->ai_family, SOCK_STREAM, 0);
	if (logSockSignaling < 0)
		ReportErrorAndExit("createSignalingLogChannel","Cannot create socket logSockSignaling!", programName, 0);
	
	SET_PORT(logHost, htons(DEFAULT_LOG_PORT_SIGNALING));
	
	signal.protocol = protocolLog;
	
	strcpy(signal.logFile, logFile);
	
	if (connect(logSockSignaling, logHost->ai_addr, logHost->ai_addrlen) < 0)
		ReportErrorAndExit("createSignalingLogChannel","Error into connect !", programName, 0);
	
	do {
		if (recv(logSockSignaling, (char *) &buffer, sizeof(BYTE), 0) < 0)
		ReportErrorAndExit("createSignalingLogChannel","Error into receive!", programName, 0);
	
  	} while (*(BYTE *) buffer != LOG_CONNECT);
	PRINTD(1,"createSignalingLogChannel: Received LOG_CONNECT Message\n");

	
	if (send(logSockSignaling, (char *) &signal, sizeof(signal), 0) < 0)
		ReportErrorAndExit("createSignalingLogChannel","Cannot send infos to LogServer !", programName, 0);

	PRINTD(1,"createSignalingLogChannel: Sent to LogServer infos for creating signaling channel\n");
}

