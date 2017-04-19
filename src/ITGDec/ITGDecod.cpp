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


#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include "../common/debug.h"
#include "../common/ITG.h"

using namespace std;

#ifdef V23
#define ADDRESSLEN 16
#else
#if V21
#pragma pack(1)
#define ADDRESSLEN 15
#else
#define ADDRESSLEN 46
#endif
#endif

#define MaxNumFlow 1000
#define fileNameSize 200

typedef char address[ADDRESSLEN];

struct stflowinfo {
	unsigned int flowId;
	double byterecv;
	unsigned long int pktrecv;
	double avdelay;
	double delvariation;
	double secfirstline;
	double seclastline;
	double maxdelay;
	double mindelay;
	address srcaddress;
	unsigned int srcport;
	address destaddress;
	unsigned int destport;
	double lastdelay;
	double avjitter;
	unsigned long int pktloss;
	unsigned int lastpktnum;
	unsigned int pktdup;
	unsigned int maxSeq;
	unsigned int minSeq;
	unsigned int lossEvents;
};

struct gestioneFileLog {
	unsigned int flowId;
	unsigned int srcPort;
	unsigned int destPort;
	address destAddr;
	address srcAddr;
	ofstream FileId;
	FILE *tempfile;
	FILE *tempfile_bit;
	FILE *tempfile_jit;
	FILE *tempfile_del;
	FILE *tempfile_pkt;
	char FileName[fileNameSize];
	char FileNWE[fileNameSize];
};

















double deltatime(double time1, double time2);
ofstream *FlowPresent(unsigned int flowId, char *destAddr, char *srcAddr, unsigned int srcPort, unsigned int destPort);
struct info *readline(ifstream *fbin, FILE *ftxt, int filetype);
struct info* found(unsigned int flowId, char *srcAddr, char *destAddr, unsigned int srcPort, unsigned int destPort, unsigned int seqNum);
void split();
void writeline(int filetype);
void printhelp();
void setline(double txTime, double rxTime);
void elabsplit(int flagbit, int flagjit, int flagdel, int flagpkt);
void merge(int type);
void mergeCombined();
void mark();


ifstream inbin;
ofstream outbin;
ifstream sendlog;
FILE *intxt;
FILE *outtxt;
int size, flagfilter, flagwinlin;
unsigned int flownum;
double msbitrate, msjitter, mspktloss, msdelay, mscombinedstats, secfirstline_gl;
gestioneFileLog memFile[MaxNumFlow];
struct stflowinfo *flowinfo = (struct stflowinfo *) malloc(sizeof(struct stflowinfo));
struct info *infos = (struct info *) malloc(sizeof(struct info));

char senderlogname[fileNameSize];

char bitratefile[fileNameSize] = "bitrate.dat";
char jitterfile[fileNameSize] = "jitter.dat";
char pktlossfile[fileNameSize] = "packetloss.dat";
char delayfile[fileNameSize] = "delay.dat";
char combinedstatsfile[fileNameSize] = "combined_stats.dat";



int main(int argc, char *argv[])
{

	
	FILE *octavefpout = 0;
	FILE *delayfpout = 0;
	FILE *jitterfpout = 0;
	char loginname[fileNameSize];
	char userstring[10];
	char logoutname[fileNameSize];
	char octavename[fileNameSize];
	char delayfpname[fileNameSize];
	char jitterfpname[fileNameSize];
	double sectxTime = 0, secrxTime = 0, avjitter = 0, lastdelay = 0, avdelay = 0, delvariation = 0;
	int flagdelay = 0, flagjitter = 0, flagbitrate = 0, flagpktloss = 0, flagcombinedstats = 0, elab_type = 0;
	int flaglogout = 0, flagsplit = 0, flagps = 0, flagidt = 0;
	int flagoctave = 0, flagfirstline = 0, flagreconstruct = 0, flageachdj = 0;
	int logintype = 0, logouttype = 0, errornum = 0, i = 0;
	unsigned long int pkterr = 0, pktvalid = 0;
	unsigned int flownumsel = 0, z = 0;
	


	
	flagdelay = 0;
	flagjitter = 0;
	flagbitrate = 0;
	flagpktloss = 0;
	flagcombinedstats = 0;
	elab_type = 0;
	flaglogout = 0;
	flagoctave = 0;
	flagfilter = 0;
	flagsplit = 0;
	flagidt = 0;
	flagps = 0;
	flagreconstruct = 0;
	flagwinlin = 0;
	flageachdj = 0;
	flownum = 0;
	pkterr = 0;
	pktvalid = 0;
	logintype = 0;
	logouttype = 0;
	flagfirstline = 1;
	avjitter = 0;
	secfirstline_gl = 90000;
	avdelay = 0;
	delvariation = 0;

	printVersion("ITGDec");

	
	if (argc <= 1) {
		printf("\nMissing filename!!!\nTry ITGDec -h or --help for more informations\n");
		exit(1);
	} else if (argv[1][0] == '-' && (argv[1][1] == 'h' || argv[1][1] == '-')) {
		printhelp();
		exit(1);
	}
	strncpy(loginname, argv[1], fileNameSize);
	argv += 2;
	argc -= 2;
	while (argc > 0) {
		if (argv[0][0] == '-') {
			if (argv[0][1] == '\0') {
				printf("Invalid option!!\n");
				printf("Use -h or --help for usage\n");
				exit(1);
			} else {
				switch (argv[0][1]) {
				case 'd':
					if ((argc < 2) || (argv[1][0] == '-')) {
						printf("Error 1 on delay time\n");
						argc -= 1;
						argv += 1;
					} else {
						msdelay = (double) atof(argv[1]);
						if (msdelay <= 0) {
							printf("Error 2 on delay time\n");
							exit(1);
						} else {
							
							
							flagdelay = 1;
							argv += 2;
							argc -= 2;
						}
						if (argc && argv[0][0] != '-') {
							strncpy(delayfile, argv[0], fileNameSize);
							argc--;
							argv++;
						}
					}
					break;
				case 'b':
					if ((argc < 2) || (argv[1][0] == '-')) {
						printf("Error 1 on bitrate time\n");
						argc -= 1;
						argv += 1;
					} else {
						msbitrate = (double) atof(argv[1]);
						if (msbitrate <= 0) {
							printf("Error 2 on bitrate time\n");
							exit(1);
						} else {
							flagbitrate = 1;
							argv += 2;
							argc -= 2;
						}
						if (argc && argv[0][0] != '-') {
							strncpy(bitratefile, argv[0], fileNameSize);
							argc--;
							argv++;
						}
					}
					break;
				case 'j':
					if ((argc < 2) || (argv[1][0] == '-')) {
						printf("Error 1 on jitter time\n");
						argc -= 1;
						argv += 1;
					} else {
						msjitter = (double) atof(argv[1]);
						if (msjitter <= 0) {
							printf("Error 2 on jitter time\n");
							exit(1);
						} else {
							flagjitter = 1;
							argv += 2;
							argc -= 2;
						}
						if (argc && argv[0][0] != '-') {
							strncpy(jitterfile, argv[0], fileNameSize);
							argc--;
							argv++;
						}
					}
					break;
				case 'p':
					if ((argc < 2) || (argv[1][0] == '-')) {
						printf("Error 1 on packet loss time\n");
						argc -= 1;
						argv += 1;
					} else {
						mspktloss = (double) atof(argv[1]);
						if (mspktloss <= 0) {
							printf("Error 2 on packet loss time\n");
							exit(1);
						} else {
							flagpktloss = 1;
							argv += 2;
							argc -= 2;
						}
						if (argc && argv[0][0] != '-') {
							strncpy(pktlossfile, argv[0], fileNameSize);
							argc--;
							argv++;
						}
					}
					break;
				case 'c':
					if ((argc < 2) || (argv[1][0] == '-')) {
						printf("Error 1 on interval value\n");
						argc -= 1;
						argv += 1;
					} else {
						mscombinedstats = (double) atof(argv[1]);
						if (mscombinedstats <= 0) {
							printf("Error 2 on interval value\n");
							exit(1);
						} else {
							
							flagdelay = 1; msdelay = mscombinedstats;
							flagbitrate = 1; msbitrate = mscombinedstats;
							flagjitter = 1; msjitter = mscombinedstats;
							flagpktloss = 1; mspktloss = mscombinedstats;
							
							
							flagcombinedstats = 1;
							argv += 2;
							argc -= 2;
						}
						if (argc && argv[0][0] != '-') {
							strncpy(combinedstatsfile, argv[0], fileNameSize);
							argc--;
							argv++;
						}
					}
					break;				
				case 'h':
					printhelp();
					exit(1);
					break;
				case '-':
					printhelp();
					exit(1);
					break;
				case 'v':
					elab_type = 1;
					argv += 1;
					argc -= 1;
					break;
				case 't':
					logintype = 1;
					argv += 1;
					argc -= 1;
					break;
				case 'w':
					flagwinlin = 1;
					argv += 1;
					argc -= 1;
					break;
				case 'P':
					flagps = 1;
					argv += 1;
					argc -= 1;
					break;
				case 'I':
					flagidt = 1;
					argv += 1;
					argc -= 1;
					break;
				case 'l':
					flaglogout = 1;
					if ((argc < 2) || (argv[1][0] == '-')) {
						printf("Error on output log file name\n");
						exit(1);
					} else {
						strncpy(logoutname, argv[1], fileNameSize);
						argc -= 2;
						argv += 2;
					}
					break;
				case 'r':
					flagreconstruct = 1;
					flaglogout = 1;
					if ((argc < 2) || (argv[1][0] == '-')) {
						printf("Error on sender log file name\n");
						exit(1);
					} else {
						strncpy(senderlogname, argv[1], fileNameSize);
						logouttype = 1;
						argc -= 2;
						argv += 2;
					}
					if (argc && argv[0][0] != '-') {
						strncpy(logoutname, argv[0], fileNameSize);
						argc--;
						argv++;
					} else {
						strncpy(logoutname, "reconstructed.dat", fileNameSize);
					}
					break;
				case 'x':
					flageachdj = 1;
					strcpy(delayfpname, "eachdelay");
					strcpy(jitterfpname, "eachjitter");
					if ((argc < 2) || (argv[1][0] == '-')) {
						strcat(delayfpname, ".dat");
						strcat(jitterfpname, ".dat");
						argc -= 1;
						argv += 1;
					} else {
						strcat(delayfpname, "_");
						strcat(jitterfpname, "_");
						strcat(delayfpname, argv[1]);
						strcat(jitterfpname, argv[1]);
						strcat(delayfpname, ".dat");
						strcat(jitterfpname, ".dat");
						argc -= 2;
						argv += 2;
					}
					break;
				case 's':
					flagsplit = 1;
					if ((argc < 2) || (argv[1][0] == '-')) {
						strcpy(userstring, "log");
						argc -= 1;
						argv += 1;
					} else {
						strncpy(userstring, argv[1], 10);
						argc -= 2;
						argv += 2;
					}
					break;
				case 'i':
					elab_type = 2;
					argv += 1;
					argc -= 1;
					break;
				case 'o':
					flagoctave = 1;
					if ((argc < 2) || (argv[1][0] == '-')) {
						printf("Error on output octave file name\n");
						exit(1);
					} else {
						strncpy(octavename, argv[1], fileNameSize);
						argc -= 2;
						argv += 2;
					}
					break;
				case 'f':
					if ((argc < 2) || (argv[1][0] == '-')) {
						printf("Error 1 flow number\n");
						argc -= 1;
						argv += 1;
					} else {
						if (argv[1][0] == 't') {
							flagfilter = 2;
							argv += 2;
							argc -= 2;
						} else {
							flownumsel = atoi(argv[1]);
							if (flownumsel <= 0) {
								printf("Error 2 on flow number\n");
								exit(1);
							} else {
								flagfilter = 1;
								argv += 2;
								argc -= 2;
							}
						}
					}
					break;
				default:
					printf("Invalid option!!\n");
					printf("Use -h or --help for usage\n");
					exit(1);
					elab_type = 1;
					break;
				} 
			}
		}
		else {
			printf("Invalid option!!\n");
			printf("Use -h or --help for usage\n");
			exit(1);
		}
	} 
	if (!((flagdelay) || (flagjitter) || (flagps) || (flagidt) || (flagbitrate) || (elab_type) || (flaglogout) || (flagpktloss)
		|| (flagoctave) || (flagsplit) || (flagreconstruct)))
		elab_type = 1;
	

	if (logintype == 0) {
		
		intxt = fopen(loginname, "rb"); 
		if (intxt == NULL) {
			printf("\n Error opening log file '%s'\n", loginname);
			perror("error ");
			exit(1);
		}
		
		
		
		
		
	} else {
		
		intxt = fopen(loginname, "r");
		if (intxt == NULL) {
			printf("\n Error opening log file '%s'\n", loginname);
			perror("error ");
			exit(1);
		}
	}

	if (flaglogout) {
		if (logouttype == 0) {
			
			if ((outtxt = fopen(logoutname, "w")) == NULL) {
				printf("\nError opening file '%s' for write\n", logoutname);
				perror("error ");
				exit(1);
			}
		} else {
			
			outbin.open(logoutname, ios::out | ios::binary | ios::trunc);
			if (!outbin.is_open()) {
				printf("\nError opening file '%s' for write\n", logoutname);
				perror("error ");
				exit(1);
			}
		}
	}
	if (flagoctave) {
		if ((octavefpout = fopen(octavename, "w")) == NULL) {
			printf("Error opening file '%s' for write", octavename);
			perror("error ");
			exit(1);
		}
	}
	if (flagbitrate || flagdelay || flagjitter || flagpktloss || flagsplit) {
		for (int j = 0; j < MaxNumFlow; j++)
			memFile[j].flowId = (unsigned int) -1;
	}

	if (flageachdj) {
		if ((delayfpout = fopen(delayfpname, "w")) == NULL) {
			printf("Error opening file '%s' for write", delayfpname);
			perror("error ");
			exit(1);
		}
		if ((jitterfpout = fopen(jitterfpname, "w")) == NULL) {
			printf("Error opening file '%s' for write", jitterfpname);
			perror("error ");
			exit(1);
		}
	}
	
	size = 1;
	while (size > 0) {
		infos = readline(&inbin, intxt, logintype);
		if (size > 0) { 
			mark();
			if (flagreconstruct) {
				struct info *send_info = found((*infos).flowId, (*infos).srcAddr, (*infos).destAddr, (*infos).srcPort,
					(*infos).destPort, (*infos).seqNum);
				if (send_info == NULL) {
					printf("Error, unable to find correspondent line in sender log file\n");
					exit(1);
				} else {
					(*infos).txTime1 = (*send_info).txTime1;
					(*infos).txTime2 = (*send_info).txTime2;
					(*infos).txTime3 = (*send_info).txTime3;

				}
			}
			if (flagps)
				printf("%u\n", (*infos).size);
			if (!flagfirstline)
				if (flagidt)
					printf("%lf\n", ((double) (*infos).rxTime1 * 3600 + (double) (*infos).rxTime2 * 60
						+ (*infos).rxTime3) - secrxTime);
			if (flaglogout)
				writeline(logouttype);
			if (flagoctave) {
				fprintf(octavefpout, "%7u ", (*infos).seqNum);
				fprintf(octavefpout, "%u %u %lf ", (*infos).txTime1, (*infos).txTime2, (*infos).txTime3);
				fprintf(octavefpout, "%u %u %lf ", (*infos).rxTime1, (*infos).rxTime2, (*infos).rxTime3);
				fprintf(octavefpout, "%u\n", (*infos).size);
				fflush(octavefpout);
			}

			
			errornum = 0;
			if (((*infos).txTime1 > 24) || ((*infos).txTime1 < 0))
				errornum = 1;
			if (((*infos).txTime2 > 59) || ((*infos).txTime2 < 0))
				errornum = 2;
			if (((*infos).txTime3 > (double) (59.999999)) || ((*infos).txTime3 < 0))
				errornum = 3;
			if (((*infos).rxTime1 > 24) || ((*infos).rxTime1 < 0))
				errornum = 4;
			if (((*infos).rxTime2 > 59) || ((*infos).rxTime2 < 0))
				errornum = 5;
			if (((*infos).rxTime3 > (double) (59.999999)) || ((*infos).rxTime3 < 0))
				errornum = 6;
			if ((flagfilter == 1) && ((*infos).flowId > flownumsel))
				errornum = 7;
			if (errornum > 0) { 
				pkterr++;
				
			} else {

				if (flagfilter == 2) {
					strcpy((*infos).srcAddr, "127.0.0.1");
					strcpy((*infos).destAddr, "127.0.0.1");
					(*infos).srcPort = 9999;
					(*infos).destPort = 8999;
					(*infos).flowId = 1;
				}
				pktvalid++;
				if (flagbitrate || flagjitter || flagdelay || flagpktloss || flagsplit)
					split();
				sectxTime = ((double) (*infos).txTime1 * 3600 + (double) (*infos).txTime2 * 60 + (*infos).txTime3);
				secrxTime = ((double) (*infos).rxTime1 * 3600 + (double) (*infos).rxTime2 * 60 + (*infos).rxTime3);
				if (flageachdj) {
					fprintf(delayfpout, "%lf\n", deltatime(sectxTime, secrxTime));
					if (!flagfirstline)
						fprintf(jitterfpout, "%lf\n", fabs(lastdelay - deltatime(sectxTime, secrxTime)));
				}
				
				setline(sectxTime, secrxTime);
				
				avdelay = avdelay + deltatime(sectxTime, secrxTime);
				delvariation = delvariation + ((deltatime(sectxTime, secrxTime)) * (deltatime(sectxTime, secrxTime)));
				if (secrxTime < secfirstline_gl)
					secfirstline_gl = secrxTime;
				
				if (flagfirstline) {
					flagfirstline = 0;
					lastdelay = deltatime(sectxTime, secrxTime);
				} else {
					avjitter = avjitter + fabs(lastdelay - deltatime(sectxTime, secrxTime));
					lastdelay = deltatime(sectxTime, secrxTime);
				}

			}
		} 
	} 

	if (pktvalid == 0) {
		printf("Empty log file\n");
		
	}

	if (flagbitrate || flagdelay || flagjitter || flagpktloss || flagsplit) {
		
		i = 0;
		while ((int) memFile[i].flowId != -1) {
			memFile[i].FileId.close();
			i++;
		}
	}

	if (flagbitrate || flagdelay || flagjitter || flagpktloss) {
		
		elabsplit(flagbitrate, flagjitter, flagdelay, flagpktloss);

		
		char tempfilename[fileNameSize] = "";

		if (flagbitrate && !flagcombinedstats) {
			merge(1);
		}
		if (flagjitter && !flagcombinedstats) {
			merge(2);
		}
		if (flagdelay && !flagcombinedstats) {
			merge(3);
		}
		if (flagpktloss && !flagcombinedstats) {
			merge(4);
		}
		if (flagcombinedstats) {
			mergeCombined();
		}
		
		if (flagbitrate) {
			i = 0;
			while ((int) memFile[i].flowId != -1) {
				strcpy(tempfilename, memFile[i].FileName);
				strcat(tempfilename, ".bit");
				if (remove(tempfilename) == -1) {
					printf("Unable to remove file '%s'\n", tempfilename);
					perror("error ");
				}
				i++;
			}
		}
		if (flagjitter) {
			i = 0;
			while ((int) memFile[i].flowId != -1) {
				strcpy(tempfilename, memFile[i].FileName);
				strcat(tempfilename, ".jit");
				if (remove(tempfilename) == -1) {
					printf("Unable to remove file '%s'\n", tempfilename);
					perror("error ");
				}
				i++;
			}
		}
		if (flagdelay) {
			i = 0;
			while ((int) memFile[i].flowId != -1) {
				strcpy(tempfilename, memFile[i].FileName);
				strcat(tempfilename, ".del");
				if (remove(tempfilename) == -1) {
					printf("Unable to remove file '%s'\n", tempfilename);
					perror("error ");
				}
				i++;
			}
		}
		if (flagpktloss) {
			i = 0;
			while ((int) memFile[i].flowId != -1) {
				strcpy(tempfilename, memFile[i].FileName);
				strcat(tempfilename, ".pkt");
				if (remove(tempfilename) == -1) {
					printf("Unable to remove file '%s'\n", tempfilename);
					perror("error ");
				}
				i++;
			}
		}
	}

	if ((flagbitrate || flagdelay || flagjitter || flagpktloss) && (!flagsplit)) {
		i = 0;
		while ((int) memFile[i].flowId != -1) {
			if (remove(memFile[i].FileName) == -1) {
				printf("Unable to remove file '%s'\n", memFile[i].FileName);
				perror("error ");
			}
			i++;
		}
	}
	if (flagsplit) {
		i = 0;
		char strappo[230];
		while ((int) memFile[i].flowId != -1) {
			strcpy(strappo, memFile[i].FileNWE);
			strcat(strappo, ".");
			strcat(strappo, userstring);
			strcat(strappo, ".dat");
			if (rename(memFile[i].FileName, strappo) == -1) {
				printf("Unable to rename file '%s'\n", memFile[i].FileName);
				perror("error ");
			}
			i++;
		}
	}

	if (elab_type == 1) {
		
		double seclastline = 0, pktrecv = 0, byterecv = 0;
		double maxdelay = -86400, mindelay = 90000;
		double totaltime = 0;
		unsigned int pktloss = 0, lossEvents = 0;
		for (z = 0; z < flownum; z++) {
			totaltime = deltatime(flowinfo[z].secfirstline, flowinfo[z].seclastline);
			pktrecv = pktrecv + flowinfo[z].pktrecv;
			byterecv = byterecv + flowinfo[z].byterecv;
			pktloss = pktloss + flowinfo[z].pktloss;
			lossEvents = lossEvents + flowinfo[z].lossEvents;
			if (seclastline < flowinfo[z].seclastline)
				seclastline = flowinfo[z].seclastline;
			if (maxdelay < flowinfo[z].maxdelay)
				maxdelay = flowinfo[z].maxdelay;
			if (mindelay > flowinfo[z].mindelay)
				mindelay = flowinfo[z].mindelay;
			flowinfo[z].avdelay = flowinfo[z].avdelay / (double) flowinfo[z].pktrecv;
			flowinfo[z].delvariation = flowinfo[z].delvariation / (double) flowinfo[z].pktrecv;
			flowinfo[z].delvariation = flowinfo[z].delvariation - (flowinfo[z].avdelay * flowinfo[z].avdelay);
			if (flowinfo[z].delvariation > 0)
				flowinfo[z].delvariation = sqrt(flowinfo[z].delvariation);
			else
				flowinfo[z].delvariation = 0;
			if (flowinfo[z].pktrecv > 1)
				flowinfo[z].avjitter = flowinfo[z].avjitter / (double) (flowinfo[z].pktrecv - 1);
			else
				flowinfo[z].avjitter = 0;
			printf("----------------------------------------------------------\n");
			printf("Flow number: %u\n", flowinfo[z].flowId);
			printf("From %s:%u", flowinfo[z].srcaddress, flowinfo[z].srcport);
			printf("\nTo    %s:%u\n", flowinfo[z].destaddress, flowinfo[z].destport);
			printf("----------------------------------------------------------\n");
			printf("Total time               = %13.6lf s\n", totaltime);
			printf("Total packets            = %13lu\n", flowinfo[z].pktrecv);
			printf("Minimum delay            = %13.6lf s\n", flowinfo[z].mindelay);
			printf("Maximum delay            = %13.6lf s\n", flowinfo[z].maxdelay);
			printf("Average delay            = %13.6lf s\n", flowinfo[z].avdelay);
			printf("Average jitter           = %13.6lf s\n", flowinfo[z].avjitter);
			printf("Delay standard deviation = %13.6lf s\n", flowinfo[z].delvariation);
			printf("Bytes received           = %13.0lf\n", flowinfo[z].byterecv);
			if (totaltime > 0) {
				printf("Average bitrate          = %13.6lf Kbit/s\n", flowinfo[z].byterecv * 8 / 1000 / totaltime);
				printf("Average packet rate      = %13.6lf pkt/s\n", (double) flowinfo[z].pktrecv / totaltime);
			} else {
				printf("Average bitrate          = %13.6lf Kbit/s\n", (double) 0);
				printf("Average packet rate      = %13.6lf pkt/s\n", (double) 0);
			}
			if (flowinfo[z].pktrecv > 0 && flowinfo[z].pktloss > 0) {
				printf("Packets dropped          = %13lu (%3.2lf %%)\n", flowinfo[z].pktloss, (double) 100
					* flowinfo[z].pktloss / (flowinfo[z].pktrecv + flowinfo[z].pktloss));
				printf("Average loss-burst size  = %13lf pkt\n", (double) flowinfo[z].pktloss / flowinfo[z].lossEvents);
			} else {
				printf("Packets dropped          = %13lu (%3.2lf %%)\n", (long unsigned int) 0, (double) 0);
				printf("Average loss-burst size  = %13lf pkt\n", (double) 0);
			}
			PRINTD(1,"Packets duplicated       = %13u \n",flowinfo[z].pktdup);
			PRINTD(1,"First sequence number    = %13u \n",flowinfo[z].minSeq);
			PRINTD(1,"Last sequence number     = %13u \n",flowinfo[z].maxSeq);
			PRINTD(1,"Loss Events              = %13u \n",flowinfo[z].lossEvents);
			printf("----------------------------------------------------------\n");
		}
		totaltime = deltatime(secfirstline_gl, seclastline);
		avdelay = avdelay / (double) pktrecv;
		delvariation = delvariation / (double) pktrecv;
		delvariation = delvariation - (avdelay * avdelay);
		if (delvariation > 0)
			delvariation = sqrt(delvariation);
		else
			delvariation = 0;
		if (pktrecv > 1)
			avjitter = avjitter / (pktrecv - 1);
		else
			avjitter = 0;
		printf("\n__________________________________________________________\n");
		printf("****************  TOTAL RESULTS   ******************\n");
		printf("__________________________________________________________\n");
		printf("Number of flows          = %13u\n", flownum);
		printf("Total time               = %13.6lf s\n", totaltime);
		printf("Total packets            = %13.0lf\n", pktrecv);
		printf("Minimum delay            = %13.6lf s\n", mindelay);
		printf("Maximum delay            = %13.6lf s\n", maxdelay);
		printf("Average delay            = %13.6lf s\n", avdelay);
		printf("Average jitter           = %13.6lf s\n", avjitter);
		printf("Delay standard deviation = %13.6lf s\n", delvariation);
		printf("Bytes received           = %13.0lf\n", byterecv);
		if (totaltime > 0) {
			printf("Average bitrate          = %13.6lf Kbit/s\n", byterecv * 8 / 1000 / totaltime);
			printf("Average packet rate      = %13.6lf pkt/s\n", pktrecv / totaltime);
		} else {
			printf("Average bitrate          = %13.6lf Kbit/s\n", (double) 0);
			printf("Average packet rate      = %13.6lf pkt/s\n", (double) 0);
		}
		if (pktrecv > 0 && pktloss > 0) {
			printf("Packets dropped          = %13.0u (%3.2lf %%)\n", pktloss, (double) (100 * pktloss / (pktrecv + pktloss)));
			printf("Average loss-burst size  = %13lf pkt\n", (double) pktloss / lossEvents);
		} else {
			printf("Packets dropped          = %13.0lf (%3.2lf %%)\n", (double) 0, (double) 0);
			printf("Average loss-burst size  = %13.0lf pkt\n", (double) 0);
		}
		printf("Error lines              = %13lu\n", pkterr);
		printf("----------------------------------------------------------\n");
	}

	
	if (logintype == 0)
		
		
		fclose(intxt);
	else
		fclose(intxt);
	if (flaglogout) {
		if (logouttype == 0)
			fclose(outtxt);
		else
			outbin.close();
	}
	if (flagoctave)
		fclose(octavefpout);

	if (flageachdj) {
		fclose(delayfpout);
		fclose(jitterfpout);
	}
	exit(0);
}





ofstream *FlowPresent(unsigned int flowId, char *destAddr, char *srcAddr, unsigned int srcPort, unsigned int destPort)
{
	int i = 0;
	while (i < MaxNumFlow) {
		if ((memFile[i].flowId == flowId) && (strcmp(memFile[i].srcAddr, srcAddr) == 0) && (strcmp(memFile[i].destAddr, destAddr)
			== 0) && (memFile[i].srcPort == srcPort) && (memFile[i].destPort == destPort))
			break;
		i++;
	}
	if (i != MaxNumFlow)
		return &memFile[i].FileId;
	else
		return NULL;
}



double deltatime(double time1, double time2)
{
	if (time2 < time1)
		return time2 - time1;
	else
		return time2 - time1;
}


struct info *found(unsigned int flowId, char *srcAddr, char *destAddr, unsigned int srcPort, unsigned int destPort, unsigned int seqNum)
{
	int flagfound = 0;
	struct info *infos_loc = (struct info *) malloc(sizeof(struct info));
	sendlog.open(senderlogname, ios::in | ios::binary);
	if (!sendlog.is_open()) {
		printf("\n Error opening log file '%s'\n", senderlogname);
		perror("error ");
		exit(1);
	}
	while (sendlog.good() && (!flagfound)) {
		sendlog.read((char *) infos_loc, sizeof(struct info));
		if (((*infos_loc).flowId == flowId) && (strcmp((*infos_loc).srcAddr, srcAddr) == 0) && (strcmp((*infos_loc).destAddr,
			destAddr) == 0) && ((*infos_loc).srcPort == srcPort) && ((*infos_loc).destPort == destPort) && ((*infos_loc).seqNum
			== seqNum))
			flagfound = 1;
	}
	if (flagfound) {
		sendlog.close();
		return infos_loc;
	} else {
		return NULL;
	}

}


void mark()
{
	static int i = 0;
	static char chr[] = "|\\-/";
	fprintf(stderr, "\b%c", chr[i++]);
	if (i > 3)
		i = 0;
	fflush(stderr);
} 



struct info *readline(ifstream *fbin, FILE *ftxt, int filetype)
{

	struct info *infos_loc = (struct info *) malloc(sizeof(struct info));

	if (filetype == 0) {
		
		
		size = fread(infos_loc, sizeof(struct info), 1, ftxt); 
		
		
		
		
	} else {
		
		char buffer;
		if (fscanf(ftxt, "Flow>%04u ", &(*infos_loc).flowId) != EOF) {
			fscanf(ftxt, "Seq>%7u ", &(*infos_loc).seqNum);
#ifdef V23
			fscanf(ftxt, "Src>%1c%15c/%u ", &buffer, (*infos_loc).srcAddr,&(*infos_loc).srcPort);
			fscanf(ftxt, "Dest>%1c%15c/%u ", &buffer, (*infos_loc).destAddr,&(*infos_loc).destPort);
			fscanf(ftxt, "TxTime>%ld:%ld:%lf ", &(*infos_loc).txTime1, &(*infos_loc).txTime2,&(*infos_loc).txTime3);
			fscanf(ftxt, "RxTime>%ld:%ld:%lf ", &(*infos_loc).rxTime1, &(*infos_loc).rxTime2,&(*infos_loc).rxTime3);
#else
#ifdef V21
			fscanf(ftxt, "Src>%1c%15c/%u ", &buffer, (*infos_loc).srcAddr,&(*infos_loc).srcPort);
			fscanf(ftxt, "Dest>%1c%15c/%u ", &buffer, (*infos_loc).destAddr,&(*infos_loc).destPort);
			fscanf(ftxt, "TxTime>%ld:%ld:%lf ", &(*infos_loc).txTime1, &(*infos_loc).txTime2,&(*infos_loc).txTime3);
			fscanf(ftxt, "RxTime>%ld:%ld:%lf ", &(*infos_loc).rxTime1, &(*infos_loc).rxTime2,&(*infos_loc).rxTime3);
#else
			fscanf(ftxt, "Src>%1c%45c/%u ", &buffer, (*infos_loc).srcAddr, &(*infos_loc).srcPort);
			fscanf(ftxt, "Dest>%1c%45c/%u ", &buffer, (*infos_loc).destAddr, &(*infos_loc).destPort);
			fscanf(ftxt, "txTime>%u:%u:%lf ", &(*infos_loc).txTime1, &(*infos_loc).txTime2, &(*infos_loc).txTime3);
			fscanf(ftxt, "rxTime>%u:%u:%lf ", &(*infos_loc).rxTime1, &(*infos_loc).rxTime2, &(*infos_loc).rxTime3);
#endif
#endif
			fscanf(ftxt, "Size>%5u\n", &(*infos_loc).size);
			if ((*infos_loc).flowId == 0) {
				printf("\n Error reading input file\n");
				size = 0;
			} else {
				size = 1;
			}
		} else
			size = 0;
	}
	return infos_loc;
}

void writeline(int filetype)
{
	if (filetype == 0) {
		
		fprintf(outtxt, "Flow>%04u ", (*infos).flowId);
		fprintf(outtxt, "Seq>%7u ", (*infos).seqNum);
#ifdef V23
		fprintf (outtxt, "Src>%16s/%hu ",(*infos).srcAddr,(*infos).srcPort);
		fprintf (outtxt, "Dest>%16s/%hu ",(*infos).destAddr,(*infos).destPort);
#else
		fprintf(outtxt, "Src>%46s/%hu ", (*infos).srcAddr, (*infos).srcPort);
		fprintf(outtxt, "Dest>%46s/%hu ", (*infos).destAddr, (*infos).destPort);
#endif
		fprintf(outtxt, "txTime>%u:%u:%lf ", (*infos).txTime1, (*infos).txTime2, (*infos).txTime3);
		fprintf(outtxt, "rxTime>%u:%u:%lf ", (*infos).rxTime1, (*infos).rxTime2, (*infos).rxTime3);
		fprintf(outtxt, "Size>%5u\n", (*infos).size);
	} else {
		
		outbin.write((char *) infos, sizeof(struct info));
	}
}

void setline(double txTime, double rxTime)
{
	double currentdelay = deltatime(txTime, rxTime);
	if (flownum == 0) { 
		flownum = 1;
		flowinfo[0].flowId = (*infos).flowId;
		flowinfo[0].byterecv = (*infos).size;
		flowinfo[0].pktrecv = 1;
		flowinfo[0].avdelay = currentdelay;
		flowinfo[0].delvariation = currentdelay * currentdelay;
		flowinfo[0].maxdelay = currentdelay;
		flowinfo[0].mindelay = currentdelay;
		flowinfo[0].secfirstline = rxTime;
		flowinfo[0].seclastline = rxTime;
		strncpy(flowinfo[0].srcaddress, (*infos).srcAddr, ADDRESSLEN);
		flowinfo[0].srcport = (*infos).srcPort;
		strncpy(flowinfo[0].destaddress, (*infos).destAddr, ADDRESSLEN);
		flowinfo[0].destport = (*infos).destPort;
		flowinfo[0].lastdelay = currentdelay;
		flowinfo[0].avjitter = 0;
		flowinfo[0].pktloss = 0;
		flowinfo[0].lastpktnum = (*infos).seqNum;
		flowinfo[0].pktdup = 0;
		flowinfo[0].maxSeq = infos->seqNum;
		flowinfo[0].minSeq = 1;
		flowinfo[0].lossEvents = 0;
		PRINTD(2,"Flow=%d Seq=%u pktrecv=%lu pktloss=%lu pktdup=%u\n",
			infos->flowId, infos->seqNum, flowinfo[0].pktrecv, flowinfo[0].pktloss, flowinfo[0].pktdup);
	} else {
		int found = 0;
		int index = 0;
		if (flagfilter == 2) {
			found = 1;
			index = 0;
		} else {
			for (unsigned int z = 0; z < flownum; z++)
				if ((flowinfo[z].flowId == (*infos).flowId) && (strcmp(flowinfo[z].srcaddress, (*infos).srcAddr) == 0)
					&& (strcmp(flowinfo[z].destaddress, (*infos).destAddr) == 0) && (flowinfo[z].srcport
					== (*infos).srcPort) && (flowinfo[z].destport == (*infos).destPort)) {
					found = 1;
					index = z;
				}
		}
		if (found) {
			
			flowinfo[index].byterecv += (*infos).size;
			flowinfo[index].pktrecv++;
			flowinfo[index].avdelay = flowinfo[index].avdelay + currentdelay;
			flowinfo[index].delvariation = flowinfo[index].delvariation + (currentdelay * currentdelay);
			if (currentdelay > flowinfo[index].maxdelay) {
				flowinfo[index].maxdelay = currentdelay;
			}
			if (currentdelay < flowinfo[index].mindelay)
				flowinfo[index].mindelay = currentdelay;
			if (rxTime < flowinfo[index].secfirstline)
				flowinfo[index].secfirstline = rxTime;
			if (rxTime > flowinfo[index].seclastline)
				flowinfo[index].seclastline = rxTime;
			double jittemp = fabs(flowinfo[index].lastdelay - currentdelay);
			flowinfo[index].avjitter = flowinfo[index].avjitter + jittemp;
			flowinfo[index].lastpktnum = (*infos).seqNum;
			flowinfo[index].lastdelay = currentdelay;
			if (infos->seqNum > flowinfo[index].maxSeq) {
				flowinfo[index].maxSeq = infos->seqNum;
			}
			if (infos->seqNum > (flowinfo[index].pktloss + flowinfo[index].pktrecv))
				flowinfo[index].lossEvents++;
			if ((flowinfo[index].maxSeq - flowinfo[index].minSeq + 1) < (flowinfo[index].pktrecv - flowinfo[index].pktdup)) {
				flowinfo[index].pktdup = -flowinfo[index].maxSeq + flowinfo[index].minSeq + flowinfo[index].pktrecv - 1;
				flowinfo[index].pktloss = 0;
			} else {
				
				if ((flowinfo[index].maxSeq - flowinfo[index].minSeq - flowinfo[index].pktrecv + flowinfo[index].pktdup + 1)
					< flowinfo[index].pktloss)
					flowinfo[index].lossEvents--;

				flowinfo[index].pktloss = flowinfo[index].maxSeq - flowinfo[index].minSeq - flowinfo[index].pktrecv
					+ flowinfo[index].pktdup + 1;
			}
			PRINTD(2,"Flow=%d Seq=%d pktrecv=%ld pktloss=%ld pktdup=%d lossEvents=%d\n",
				infos->flowId, infos->seqNum, flowinfo[index].pktrecv, flowinfo[index].pktloss,
				flowinfo[index].pktdup, flowinfo[index].lossEvents);
		} else {
			
			flownum++;
			flowinfo = (struct stflowinfo *) realloc(flowinfo, (flownum + 1) * sizeof(struct stflowinfo));
			flowinfo[flownum - 1].flowId = (*infos).flowId;
			flowinfo[flownum - 1].byterecv = (*infos).size;
			flowinfo[flownum - 1].pktrecv = 1;
			flowinfo[flownum - 1].avdelay = currentdelay;
			flowinfo[flownum - 1].delvariation = currentdelay * currentdelay;
			flowinfo[flownum - 1].maxdelay = currentdelay;
			flowinfo[flownum - 1].mindelay = currentdelay;
			flowinfo[flownum - 1].secfirstline = rxTime;
			flowinfo[flownum - 1].seclastline = rxTime;
			strncpy(flowinfo[flownum - 1].srcaddress, (*infos).srcAddr, ADDRESSLEN);
			flowinfo[flownum - 1].srcport = (*infos).srcPort;
			strncpy(flowinfo[flownum - 1].destaddress, (*infos).destAddr, ADDRESSLEN);
			flowinfo[flownum - 1].destport = (*infos).destPort;
			flowinfo[flownum - 1].lastdelay = currentdelay;
			flowinfo[flownum - 1].avjitter = 0;
			flowinfo[flownum - 1].pktloss = 0;
			flowinfo[flownum - 1].lastpktnum = (*infos).seqNum;
			flowinfo[flownum - 1].pktdup = 0;
			flowinfo[flownum - 1].maxSeq = infos->seqNum;
			flowinfo[flownum - 1].minSeq = 1;
			flowinfo[flownum - 1].lossEvents = 0;
			PRINTD(2,"Flow=%d Seq=%d pktrecv=%ld pktloss=%ld pktdup=%d lossEvents=%d\n",
				infos->flowId, infos->seqNum, flowinfo[flownum-1].pktrecv,
				flowinfo[flownum-1].pktloss, flowinfo[flownum-1].pktdup, flowinfo[flownum-1].lossEvents);
		}
	}
}


void split()
{
	int i = 0;
	ofstream *IdFile;
	
	IdFile = FlowPresent((*infos).flowId, (*infos).destAddr, (*infos).srcAddr, (*infos).srcPort, (*infos).destPort);
	if (IdFile != NULL) { 
		(*IdFile).write((char *) infos, sizeof(struct info));
		if (!(*IdFile)) {
			printf("Unable to write file \n");
			perror("Split : ");
			exit(1);
		}
	} else {
		i = 0; 
		while ((memFile[i].flowId != (unsigned int) -1) && (i < MaxNumFlow)) {
			i++;
		}
		char buffer1[fileNameSize] = "";
		char buffer2[fileNameSize] = "";
		char* nameFileLog = buffer2;
		char flowIdString[33];
		sprintf(flowIdString, "%u", (*infos).flowId);
		strcat(nameFileLog, flowIdString);
		strcat(nameFileLog, "-");
		strcat(nameFileLog, (*infos).srcAddr);
		strcat(nameFileLog, "-");
		strcat(nameFileLog, (*infos).destAddr);
		strcpy(buffer1, buffer2);
		strcat(nameFileLog, ".dat");

		memFile[i].FileId.open(buffer2, ios::out | ios::binary | ios::trunc);
		if (!memFile[i].FileId.is_open()) {
			printf("Unable to open file %s \n", buffer2);
			perror("ITGSplit main: ");
			exit(1);
		}
		
		memFile[i].flowId = (*infos).flowId;
		memFile[i].srcPort = (*infos).srcPort;
		memFile[i].destPort = (*infos).destPort;
		strncpy(memFile[i].srcAddr, (*infos).srcAddr, ADDRESSLEN);
		strncpy(memFile[i].destAddr, (*infos).destAddr, ADDRESSLEN);
		strcpy(memFile[i].FileName, buffer2);
		strcpy(memFile[i].FileNWE, buffer1);
		memFile[i].FileId.write((char *) infos, sizeof(struct info));
		if (!memFile[i].FileId) {
			printf("Unable to write into file %s \n", buffer2);
			perror("Split main: ");
			exit(1);
		}
	} 
}

void elabsplit(int flagbit, int flagjit, int flagdel, int flagpkt)
{
	
	

	int i = 0, flagfirstline;
	double secrxTime, secfirstline_loc = 90000;
	double secinitbitrate = 0, tempbitrate = 0;
	double secinitjitter = 0, tempjitter = 0, lastdelay = 0, currentdelay = 0;
	double secinitdelay = 0, tempdelay = 0;
	double secinitpktloss = 0;
	unsigned long int intclosedbit = 0, intclosedjit = 0, intcloseddel = 0, intclosedpkt = 0, z = 0;
	unsigned long int pktjitter = 0, pktdelay = 0;
	unsigned long int pktrecv = 0, pktloss = 0, pktdup = 0;
	unsigned long int temppktloss = 0, pktpktloss = 0, totalpktloss = 0;
	
	
	FILE *inputfilec = 0;
	FILE *outfilebit = 0;
	FILE *outfilejit = 0;
	FILE *outfiledel = 0;
	FILE *outfilepkt = 0;
	char outfilename[fileNameSize];
	

	while (memFile[i].flowId != (unsigned int) -1) {

		
		if (flagbit) {
			strcpy(outfilename, "");
			strcpy(outfilename, memFile[i].FileName);
			strcat(outfilename, ".bit");
			if ((outfilebit = fopen(outfilename, "w")) == NULL) {
				printf("Error opening file %s for write \n", outfilename);
				perror("error ");
				exit(1);
			}
		}
		if (flagjit) {
			strcpy(outfilename, "");
			strcpy(outfilename, memFile[i].FileName);
			strcat(outfilename, ".jit");
			if ((outfilejit = fopen(outfilename, "w")) == NULL) {
				printf("Error opening file %s for write \n", outfilename);
				perror("error ");
				exit(1);
			}
		}
		if (flagdel) {
			strcpy(outfilename, "");
			strcpy(outfilename, memFile[i].FileName);
			strcat(outfilename, ".del");
			if ((outfiledel = fopen(outfilename, "w")) == NULL) {
				printf("Error opening file %s for write \n", outfilename);
				perror("error ");
				exit(1);
			}
		}
		if (flagpkt) {
			strcpy(outfilename, "");
			strcpy(outfilename, memFile[i].FileName);
			strcat(outfilename, ".pkt");
			if ((outfilepkt = fopen(outfilename, "w")) == NULL) {
				printf("Error opening file %s for write \n", outfilename);
				perror("error ");
				exit(1);
			}
		}
		
		
		
		
		inputfilec = fopen(memFile[i].FileName, "rb");
		PRINTD(1,"File %s aperto\n", memFile[i].FileName);
		if (inputfilec == 0) {
			printf("Error opening file for aftersplit \n");
			perror("error ");
			exit(1);
		} else {
			if (flagbit) {
				intclosedbit = 0;
				tempbitrate = 0;
			}
			if (flagjit) {
				intclosedjit = 0;
				tempjitter = 0;
				pktjitter = 0;
			}
			if (flagdel) {
				pktdelay = 0;
				intcloseddel = 0;
				tempdelay = 0;
			}
			if (flagpkt) {
				intclosedpkt = 0;
				temppktloss = 0;
				pktpktloss = 0;
				totalpktloss = 0;
			}
			flagfirstline = 0;
			pktloss = 0;
			pktdup = 0;
			pktrecv = 0;
			while (1) { 
				
				
				
				PRINTD(1,"Prima di leggere le info dal file %s con puntantore %p\n",
					memFile[i].FileName, inputfilec);

				size = fread(infos, sizeof(struct info), 1, inputfilec); 
				PRINTD(1,"Info lette dal file %s\n", memFile[i].FileName);
				if (size == 0) {
					if (flagbit)
						fprintf(outfilebit, "%lf %lf\n", intclosedbit * (double) msbitrate / 1000, tempbitrate * 8
							/ msbitrate);
					if (flagjit)
						fprintf(outfilejit, "%lf %lf\n", intclosedjit * (double) msjitter / 1000, tempjitter
							/ (double) (pktjitter - 1));
					if (flagdel)
						fprintf(outfiledel, "%lf %lf\n", intcloseddel * (double) msdelay / 1000, tempdelay
							/ (double) pktdelay);
					if (flagpkt)
						fprintf(outfilepkt, "%lf %lu\n", intclosedpkt * (double) mspktloss / 1000, temppktloss);
					break;
				} else { 
					pktrecv++;
					if ((*infos).seqNum > (pktrecv + pktloss - pktdup)) {
						pktloss = (*infos).seqNum - pktrecv;
					} else if ((*infos).seqNum < (pktrecv + pktloss - pktdup)) {
						pktdup++;
						
					}
					secrxTime = (double) ((*infos).rxTime1 * 3600) + (double) ((*infos).rxTime2 * 60)
						+ (*infos).rxTime3;
					currentdelay = deltatime(((double) (*infos).txTime1 * 3600) + ((double) (*infos).txTime2 * 60)
						+ ((*infos).txTime3), secrxTime);
					if (!flagfirstline) {
						flagfirstline = 1;
						secfirstline_loc = secrxTime;
						if (flagbit) {
							secinitbitrate = secfirstline_gl;
						}
						if (flagjit) {
							secinitjitter = secfirstline_gl;
							lastdelay = currentdelay;
						}
						if (flagdel) {
							secinitdelay = secfirstline_gl;
						}
						if (flagpkt) {
							secinitpktloss = secfirstline_gl;
						}
					}
					if (flagbit) {
						if ((secrxTime - secinitbitrate) > (msbitrate / 1000)) {
							
							double temporary = deltatime(secfirstline_gl, secrxTime) * 1000 / msbitrate;
							unsigned long int inttoclose = (unsigned long int) (ceil(temporary) - intclosedbit
								- 1);
							if (inttoclose > 0) {
								
								for (z = 1; z <= inttoclose; z++) {
									
									if (((double) ((intclosedbit + 1) * msbitrate / 1000)
										+ secfirstline_gl) < secfirstline_loc) {
										fprintf(outfilebit, "%lf %lf\n", intclosedbit
											* (double) msbitrate / 1000, (double) 0);
									} else {
										fprintf(outfilebit, "%lf %lf\n", intclosedbit
											* (double) msbitrate / 1000, tempbitrate * 8
											/ msbitrate);
										tempbitrate = 0;
									}
									intclosedbit++;
								}
							}
							secinitbitrate = (intclosedbit * msbitrate) / 1000 + secfirstline_gl;
						} 
						
						tempbitrate += (double) (*infos).size;
					}

					if (flagjit) {
						if ((secrxTime - secinitjitter) > (msjitter / 1000)) {
							
							double temporary = deltatime(secfirstline_gl, secrxTime) * 1000 / msjitter;
							unsigned long int inttoclose = (unsigned long int) (ceil(temporary) - intclosedjit
								- 1);
							if (inttoclose > 0) {
								
								for (z = 1; z <= inttoclose; z++) {
									
									if (((double) ((intclosedjit + 1) * msjitter / 1000)
										+ secfirstline_gl) < secfirstline_loc) {
										fprintf(outfilejit, "%lf %lf\n", intclosedjit
											* (double) msjitter / 1000, (double) 0);
									} else {
										if (pktjitter > 1) {
											fprintf(outfilejit, "%lf %lf\n", intclosedjit
												* (double) msjitter / 1000, tempjitter
												/ (double) (pktjitter - 1));
											pktjitter = 1;
										} else
											fprintf(outfilejit, "%lf %lf\n", intclosedjit
												* (double) msjitter / 1000, (double) 0);
										tempjitter = 0;
										
									}
									intclosedjit++;
								}
							}
							secinitjitter = intclosedjit * msjitter / 1000 + secfirstline_gl;
						} 
						tempjitter = tempjitter + fabs(currentdelay - lastdelay);
						pktjitter++;
						lastdelay = currentdelay;
					}

					if (flagdel) {
						if ((secrxTime - secinitdelay) > (msdelay / 1000)) {
							
							double temporary = deltatime(secfirstline_gl, secrxTime) * 1000 / msdelay;
							unsigned long int inttoclose = (unsigned long int) (ceil(temporary) - intcloseddel
								- 1);
							if (inttoclose > 0) {
								
								for (z = 1; z <= inttoclose; z++) {
									
									if (((double) ((intcloseddel + 1) * (msdelay / 1000))
										+ secfirstline_gl) < secfirstline_loc)
										fprintf(outfiledel, "%lf %lf\n", intcloseddel
											* (double) msdelay / 1000, (double) 0);
									else {
										if (pktdelay > 0) {
											fprintf(outfiledel, "%lf %lf\n", intcloseddel
												* (double) msdelay / 1000, tempdelay
												/ (double) pktdelay);
											tempdelay = 0;
											pktdelay = 0;
										} else {
											fprintf(outfiledel, "%lf %lf\n", intcloseddel
												* (double) msdelay / 1000, (double) 0);
										}
									}
									intcloseddel++;
								}
							}
							secinitdelay = intcloseddel * msdelay / 1000 + secfirstline_gl;
						} 
						tempdelay = tempdelay + currentdelay;
						pktdelay++;
					}

					if (flagpkt) {
						if ((secrxTime - secinitpktloss) > (mspktloss / 1000)) {
							
							double temporary = deltatime(secfirstline_gl, secrxTime) * 1000 / mspktloss;
							unsigned long int inttoclose = (unsigned long int) (ceil(temporary) - intclosedpkt
								- 1);
							if (inttoclose > 0) {
								
								for (z = 1; z <= inttoclose; z++) {
									
									if (((double) ((intclosedpkt + 1) * mspktloss / 1000)
										+ secfirstline_gl) < secfirstline_loc)
										fprintf(outfilepkt, "%lf %lu\n", intclosedpkt
											* (double) mspktloss / 1000, (unsigned long int) 0);
									else {
										fprintf(outfilepkt, "%lf %lu\n", intclosedpkt
											* (double) mspktloss / 1000, temppktloss);
										temppktloss = 0;
									}
									intclosedpkt++;
								}
							}
							secinitpktloss = intclosedpkt * mspktloss / 1000 + secfirstline_gl;
						} 
						pktpktloss++;
						if ((*infos).seqNum > (pktpktloss + totalpktloss)) {
							temppktloss = temppktloss + (*infos).seqNum - pktpktloss - totalpktloss;
							totalpktloss = (*infos).seqNum - pktpktloss;
						}
					}
				}
			}
		}
		i++;
		
		
		fclose(inputfilec);
		
		if (flagbit)
			fclose(outfilebit);
		if (flagjit)
			fclose(outfilejit);
		if (flagdel)
			fclose(outfiledel);
		if (flagpkt)
			fclose(outfilepkt);
	}
}


void merge(int type)
{
	

	int i = 0, finishedfiles, flagfirstfile;
	unsigned int flagbreak;
	FILE *fpout = 0;
	double time, value, valuesum;
	char extension[5] = "";
	char colname[fileNameSize] = "";

	
	switch (type) {
	case 1:
		strcpy(extension, ".bit\0");
		if ((fpout = fopen(bitratefile, "w")) == NULL) {
			printf("Error opening file 'bitrate.dat' for write");
			perror("error ");
			exit(1);
		}
		break;
	case 2:
		strcpy(extension, ".jit\0");
		if ((fpout = fopen(jitterfile, "w")) == NULL) {
			printf("Error opening file 'jitter.dat' for write");
			perror("error ");
			exit(1);
		}
		break;
	case 3:
		strcpy(extension, ".del\0");
		if ((fpout = fopen(delayfile, "w")) == NULL) {
			printf("Error opening file 'delay.dat' for write");
			perror("error ");
			exit(1);
		}
		break;
	case 4:
		strcpy(extension, ".pkt\0");
		if ((fpout = fopen(pktlossfile, "w")) == NULL) {
			printf("Error opening file 'pktloss.dat' for write");
			perror("error ");
			exit(1);
		}
		break;
	}

	fprintf(fpout, "Time");
	
	while (memFile[i].flowId != (unsigned int) -1) {
		char tempfilename[fileNameSize] = "";
		strcat(tempfilename, memFile[i].FileName);
		strcat(tempfilename, extension);
		if ((memFile[i].tempfile = fopen(tempfilename, "r")) == NULL) {
			printf("\n Error opening log file '%s'\n", tempfilename);
			perror("error ");
			exit(1);
		}
		
		sprintf(colname, "%u", memFile[i].flowId);
		strcat(colname, "-");
		strcat(colname, memFile[i].srcAddr);
		strcat(colname, "-");
		strcat(colname, memFile[i].destAddr);
		fprintf(fpout, " %s", colname);
		strcpy(colname, "");

		i++;
	}
	fprintf(fpout, " Aggregate-Flow\n");
	while (1) {
		flagfirstfile = 0;
		i = 0;
		flagbreak = 0;
		finishedfiles = 0;
		valuesum = 0;
		while (memFile[i].flowId != (unsigned int) -1) {
			if (fscanf(memFile[i].tempfile, "%lf %lf\n", &time, &value) == EOF) {
				flagbreak++;
				finishedfiles++;
			} else {
				valuesum = valuesum + value;
				if (!flagfirstfile) {
					flagfirstfile = 1;
					fprintf(fpout, "%lf", time);
				}
				while (finishedfiles > 0) { 
					fprintf(fpout, " %lf", (double) 0);
					finishedfiles--;
				}
				fprintf(fpout, " %lf", value);
			}
			i++;
			if ((memFile[i].flowId == (unsigned int) -1) && (flagbreak != flownum)) {
				
				while (finishedfiles > 0) { 
					fprintf(fpout, " %lf", (double) 0);
					finishedfiles--;
				}
				fprintf(fpout, " %lf\n", valuesum);
			}
		} 
		if (flagbreak == flownum)
			break;
	} 
	i = 0;
	while (memFile[i].flowId != (unsigned int) -1) {
		fclose(memFile[i].tempfile);
		i++;
	}
	fclose(fpout);
}

void mergeCombined() {
	int i = 0, flagfirstfile;
	unsigned int flagbreak;
	FILE *fpout = 0;
	double time, value, valuesum_bit, valuesum_del, valuesum_jit, valuesum_pkt;
	char tempfilename[fileNameSize] = "";

	if ((fpout = fopen(combinedstatsfile, "w")) == NULL) {
		printf("Error opening file 'combinedstatsfile.txt' for write");
		perror("error ");
		exit(1);
	}

	while (memFile[i].flowId != (unsigned int) -1) {
		strcpy(tempfilename, "");
		strcat(tempfilename, memFile[i].FileName);
		strcat(tempfilename, ".bit\0");
		if ((memFile[i].tempfile_bit = fopen(tempfilename, "r")) == NULL) {
			printf("\n Error opening log file '%s'\n", tempfilename);
			perror("error ");
			exit(1);
		}

		strcpy(tempfilename, "");
		strcat(tempfilename, memFile[i].FileName);
		strcat(tempfilename, ".del\0");
		if ((memFile[i].tempfile_del = fopen(tempfilename, "r")) == NULL) {
			printf("\n Error opening log file '%s'\n", tempfilename);
			perror("error ");
			exit(1);
		}

		strcpy(tempfilename, "");
		strcat(tempfilename, memFile[i].FileName);
		strcat(tempfilename, ".jit\0");
		if ((memFile[i].tempfile_jit = fopen(tempfilename, "r")) == NULL) {
			printf("\n Error opening log file '%s'\n", tempfilename);
			perror("error ");
			exit(1);
		}

		strcpy(tempfilename, "");
		strcat(tempfilename, memFile[i].FileName);
		strcat(tempfilename, ".pkt\0");
		if ((memFile[i].tempfile_pkt = fopen(tempfilename, "r")) == NULL) {
			printf("\n Error opening log file '%s'\n", tempfilename);
			perror("error ");
			exit(1);
		}	
		i++;
	}
	
	
	
			
	while (1) {
		flagfirstfile = 0;
		i = 0;
		flagbreak = 0;
		valuesum_bit = 0;
		valuesum_del = 0;
		valuesum_jit = 0;
		valuesum_pkt = 0;
		while (memFile[i].flowId != (unsigned int) -1) {

			value = -1;
			if (fscanf(memFile[i].tempfile_bit, "%lf %lf\n", &time, &value) == EOF) {
				flagbreak++;
			} else {
				if (!flagfirstfile) {
					flagfirstfile = 1;
					fprintf(fpout, "%lf", time);
				}
				valuesum_bit += value;
							
				
				value = -1;
				fscanf(memFile[i].tempfile_del, "%lf %lf\n", &time, &value);
				valuesum_del += value;

				
				value = -1;
				fscanf(memFile[i].tempfile_jit, "%lf %lf\n", &time, &value);
				valuesum_jit += value;
				
				
				value = -1;
				fscanf(memFile[i].tempfile_pkt, "%lf %lf\n", &time, &value);
				valuesum_pkt += value;				
			}
			i++;
			
			if ((memFile[i].flowId == (unsigned int) -1) && (flagbreak != flownum)) {	
				fprintf(fpout, " %lf %lf %lf %lf\n", valuesum_bit, valuesum_del, valuesum_jit, valuesum_pkt);
			}
		} 
		if (flagbreak == flownum)
			break;
	} 
	
	
	i = 0;
	while (memFile[i].flowId != (unsigned int) -1) {
		fclose(memFile[i].tempfile_bit);
		fclose(memFile[i].tempfile_del);
		fclose(memFile[i].tempfile_jit);
		fclose(memFile[i].tempfile_pkt);
		i++;
	}
	fclose(fpout);
}

void printhelp()
{
	cout << "\nITGDec - Decoder component of D-ITG platform\n";

	cout << "\n Synopsis\n\n";

	cout << "     ./ITGDec <logfile> [options]\n";

	cout << "\n Options\n\n"

		"    -v                   Print standard summary to stdout (default).\n\n"

		"    -l <txtlog>          Print to <txtlog> the decoded log in text format.\n\n"

		"    -t                   Interpret <logfile> as in text format.\n\n"

		"    -o <outfile>         Print to <outfile> the decoded log for Octave/Matlab import.\n\n"

		"    -r <sender_log>      Generate combined log file starting from receiver- and sender-side log files\n"
		"       [combined_log]    (default filename: 'combined.dat').\n\n"

		"    -d <DT> [filename]   Print average delay to file every <DT> milliseconds\n"
		"                         (default filename: 'delay.dat').\n\n"

		"    -j <JT> [filename]   Print average jitter to file every <JT> milliseconds\n"
		"                         (default filename: 'jitter.dat').\n\n"

		"    -b <BT> [filename]   Print average bitrate to file every <BT> milliseconds\n"
		"                         (default filename: 'bitrate.dat').\n\n"

		"    -p <PT> [filename]   Print average packet loss to file every <PT> milliseconds\n"
		"                         (default filename: 'packetloss.dat').\n\n"

		"    -c <CT> [filename]   Print all average metrics to file every <CT> milliseconds\n"
		"                         (default filename: 'combined_stats.dat').\n\n"

		"    -s <suffix>          Generate a separate log file for each flow adding a suffix to its name\n"
                "                         (default suffix: log).\n\n"

		"    -f <flownum>         Consider only flows with number <= <flownum>.\n"
		"                         Setting <flownum> to 't' all packets are considered as part of the same flow.\n\n"

		"    -P                   Print to stdout the size of each packet.\n\n"

		"    -I                   Print to stdout the inter departure time between each packet pair.\n\n"


		"    -h | --help          Display this help and exit\n\n";

	cout << "\nFor more information please refer to the manual.\n";

	
	
	
}
