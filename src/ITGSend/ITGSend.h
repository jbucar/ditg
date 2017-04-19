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



#include <vector>					
#include <limits.h>
using namespace std;				
#define DIM_NOME_FILE 200			
#define DIM_MAX_IP_PACKET 65536		

#define DefaultDSByte 0
#define rttmDelay 15
#define MAX_SCRIPT_LINE_SIZE 400


#define MSG_SM_NEWFLOW 1
#define MSG_SM_ENDFLOW 2
#define MSG_SM_ERRFLOW 3
#define MSG_SM_SNDACK  4	


#define MSG_FP_END 1
#define MSG_FP_ERR1 2
#define MSG_FP_ERR2 3
#define MSG_FP_ERR3 4
#define MSG_FP_ERR4 5


#ifdef UNIX
extern const char DefaultLogFile[];
typedef int HANDLE; 
#endif
#ifdef WIN32
extern char DefaultLogFile[];
#endif
extern const char DefaultDestIP[];




struct flowDescriptor {
	int id;
	struct addrinfo *SrcHost;
	struct addrinfo *DestHost;
	bool srcPortSpecify;
	char *iface;
	BYTE meter;
	BYTE l4Proto;
	BYTE l7Proto;
	int icmptype;
	unsigned int DSByte;
	unsigned long Duration;
	int TTL;
	SumRandom * IntArriv;
	SumRandom * PktSize;
	TDistro IntArrivDistro;
	TDistro PktSizeDistro;
	
#ifdef BURSTY
	bool bursty;
	SumRandom * OnPeriod;
	SumRandom * OffPeriod;
	TDistro OnPeriodDistro;
	TDistro OffPeriodDistro;
#endif
	
	int sigChanId;
	BYTE status;
	bool Nagle;
	pthread_t handle;

	HANDLE serial;
	char serialReceiver[DIM_NAME_SERIAL_INTERFACE];

#ifdef UNIX
	int parserPipe[2]; 
#endif
#ifdef WIN32
	HANDLE parserPipe[3]; 
#endif
#ifdef SCTP
	int sctpId;
#endif

	int minPayloadSize;
	int payloadLogType;

	char payloadFile[DIM_PAYLOAD_FILE]; 		
	void * ptrPayloadBuffer; 			
	long int dimPayloadBuffer; 			

	int mean_adjustment;
	bool srcAddrSpecify; 				
	bool dstPortSpecify;				
	
	char pktSizeFile[DIM_NOME_FILE];		
	vector<uint32_t> vectSize;			
	char timeFile[DIM_NOME_FILE];			
	vector<Real> vectTime;				
	unsigned int numPacket;				

	unsigned long int KBToSend;			

	
	bool dstAddrSpecify;                  

	
	struct addrinfo *SigDestHost;   		
	struct addrinfo *SigSrcHost;			
	uint16_t SigSrcPort;		
	char *SigInterface;					

	bool pollingMode;						

	unsigned long int Delay;				
	struct timeval FlowStartTime;			
};

extern flowDescriptor flows[MAX_NUM_THREAD];


struct flowParserParams {
	int flowId;
	char line[MAX_FLOW_LINE_SIZE];
};


struct icmppkt {
	struct icmp icmp_buf;
	char packet[MAX_PAYLOAD_SIZE];
};



struct icmppktv6 {
	struct icmpv6 icmp_buf;
	char packet[MAX_PAYLOAD_SIZE];
};


struct signalChannel {
	int socket;
	int flows;
#ifdef UNIX
	int pipe[2]; 
#endif
#ifdef WIN32
	HANDLE pipe[3]; 
#endif
	pthread_t handle;
	struct addrinfo *DestAddr;
	bool errorLog;
};

extern signalChannel signalChannels[MAX_NUM_THREAD];

extern int sock;
extern char logFile[DIM_LOG_FILE];	
extern ofstream out;			
extern int logging;			
extern int logSock;
extern int logremoto;
extern struct addrinfo *logHost;


extern BYTE protoTxLog;
extern BYTE protoTx_ServerLogReceiver;
extern int logSockSignaling;
extern int multiFlows;
extern int signalChanCount;
extern struct addrinfo *serverLogReceiver;
extern char LogFileServerLogReceiver[DIM_LOG_FILE];
extern int logServer;
extern int managerMode;
extern int managerSock;
#ifdef UNIX
extern pthread_mutex_t mutex;
extern pthread_mutex_t mutexLog;
#endif
#ifdef WIN32
extern HANDLE mutex;
extern HANDLE mutex_log;
#endif


int modeManager(int argc, char *argv[]);
int modeScript(int argc, char *argv[]);
int modeCommandLine(int argc, char *argv[]);
void parserMultiFlows(char *argv[], int argc);
void *flowParser(void *para);
void *signalManager(void *id);

int identifySignalManager(int flowId, int *chanId, struct addrinfo* &DestHost,bool &dstAddrSpecify); 	
void *flowSender(void *param);
void Terminate(int sig);
void printHelp();
void ReportErrorAndExit(const char *function, const char *msg, char *pname, int fid);
void sendType(int signalSock, BYTE typeMessag);
#ifdef UNIX
void createSignalChan(int signalSock);

void createTransportChan(int &signalSock, struct addrinfo* &DestHost,bool &dstAddrSpecify,struct addrinfo* &SrcHost,uint16_t SrcPort,const char * Interface);   
void sendRelease(int signalSock);
#else
void createSignalChan(unsigned int signalSock);

void createTransportChan(unsigned int &signalSock, struct addrinfo* &DestHost,bool &dstAddrSpecify,struct addrinfo* &SrcHost,uint16_t SrcPort);        
void sendRelease(unsigned int signalSock);
#endif
int sendLog(int signalSock, struct addrinfo *loghost, BYTE logProtocol, char *FileName);
void recvAck(int signalSock);
void closedFlowErr(int flowId, int signalSock);
int closedFlow(int flowId, int signalSock);
int sendAckFlow(int flowId, int signalSock);	
int requestToSend(int flowId, int signalSock);
bool checkDestHostIP(int * chanId, struct addrinfo * DestHost);
void argvToString(char *argv[], int argc, char line[200]);
void flushBuffer(info *infos, int count);
void createRemoteLogFile(struct addrinfo *LogHost, BYTE Prototx_Log);
void memClean();
int isChannelClosable(int id);
void createSignalingLogChannel(BYTE protocolLog);
int sendNameLog(int signalSock, char *FileName,int sizeFileName);


