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
#include "newran/newran.h"	

#ifdef UNIX
#include <netdb.h>
#include <sys/wait.h>
#include <math.h>
#include <netinet/tcp.h>	
#include <netinet/ip.h>
#include <sys/shm.h>
#include <pthread.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#endif
#ifdef WIN32
#include <math.h>
#include <sys/timeb.h>
#include <time.h>
#include <signal.h>
#endif

#include "traffic.h"
#include "ITGSend.h"

const char *DistroStrings[] = { "Constant", "Uniform", "Exponential", "Pareto",
	"Cauchy", "Normal", "Poisson", "Gamma", "Weibull",
	"Telnet", "AoM", "Discrete", "Extreme_Largest", "Quake3", "Student", "Counterstrike active", "Counterstrike inactive" };

const unsigned int DefaultPktPerSec = 1000;
const unsigned int DefaultPktSize = 512;
const unsigned long DefaultDuration = 10000;
const unsigned long DefaultDelay = 0;


Constant *ConstantRV;
Uniform *UniformRV;
Exponential *ExponentialRV;
Pareto *ParetoRV;
Cauchy *CauchyRV;
Normal *NormalRV;
Poisson *PoissonRV;
Gamma *GammaRV;

Weibull *WeibullRV;

DiscreteGen *DiscreteRV;
MixedRandom *MixedRV;
Extreme_Largest *Extreme_LargestRV;
Student *StudentRV;
MixedRandom *Mixed_rv;    
DiscreteGen *Discrete_rv;  

void telnetParser(SumRandom ** pIntArriv, SumRandom ** pPktSize, TDistro & IntArrivDistro,
    TDistro & PktSizeDistro)
{
	int numval = 15;
	Real prob[] =
	    { 0.65, 0.13, 0.07, 0.05, 0.03, 0.01, 0.01, 0.01, 0.01, 0.005, 0.005, 0.005, 0.005,
		    0.005, 0.005 };
	
	ParetoRV = new Pareto(0.95);
	IntArrivDistro = pdPareto;
	delete(*pIntArriv);
	*pIntArriv = new SumRandom(1.1 * (*ParetoRV));
	
	PktSizeDistro = pdTelnet;
	delete(*pPktSize);
	static DiscreteGen D(numval, prob);
	*pPktSize = new SumRandom(D + 21);
}

void voIPParser(int h, char *argv[], int &argc, unsigned int flowId, SumRandom ** pIntArriv,
    SumRandom ** pPktSize, TDistro & IntArrivDistro, TDistro & PktSizeDistro)
{
	char codec[10];
	
	strcpy(codec, "G.711.1");
	Real samples = 1.0;
	Real framesize = 80.0;
	Real VAD = 1.0;
	Real header = 12.0;
	Real pkts = 100.0;
	argv++;
	argc--;
	while (argc > 0) {
		if (argv[h][0] == '-') {
			char *tail;
			switch (argv[h][1]) {
			case 'x':
				if (argc < 2)
					ReportErrorAndExit("VoIP parser", "Invalid Codec Type",
					    programName, flowId);
				if (strcmp(argv[h + 1], "G.711.1") == 0) {
					strcpy(codec, "G.711");
					framesize = 80.0;
					samples = 1.0;
					pkts = 100.0;
				} else if (strcmp(argv[h + 1], "G.711.2") == 0) {
					strcpy(codec, "G.711");
					framesize = 80.0;
					samples = 2.0;
					pkts = 50.0;
				} else if (strcmp(argv[h + 1], "G.729.2") == 0) {
					strcpy(codec, "G.729");
					framesize = 10.0;
					samples = 2.0;
					pkts = 50.0;
				} else if (strcmp(argv[h + 1], "G.729.3") == 0) {
					strcpy(codec, "G.729");
					framesize = 10.0;
					samples = 3.0;
					pkts = 33.0;
				} else if (strcmp(argv[h + 1], "G.723.1") == 0) {
					strcpy(codec, argv[h + 1]);
					framesize = 30.0;
					samples = 1.0;
					pkts = 26.0;
				} else
					ReportErrorAndExit("VoIP parser", "Invalid Codec Type",
					    programName, flowId);
				
				h += 2;
				argc -= 2;
				break;
			case 'h':
				if (argc < 2)
					ReportErrorAndExit("VoIP parser", "Invalid Protocol Type",
					    programName, flowId);
				if (strcmp(argv[h + 1], "RTP") == 0)
					header = 12.0;
				else if (strcmp(argv[h + 1], "CRTP") == 0)
					header = 2;
				else
					ReportErrorAndExit("VoIP parser",
					    "Invalid Protocol Type (RTP or CRTP)", programName,
					    flowId);
				h += 2;
				argc -= 2;
				break;
			case 'V':
				if ((argv[h][2] == 'A') && (argv[h][3] == 'D'))    
					VAD = 0.65;
				else
					ReportErrorAndExit("VoIP parser", "Invalid Option (VAD)",
					    programName, flowId);
				h += 1;
				argc -= 1;
				break;
			default:
				tail = (char *) malloc(sizeof("Unknown option ") + sizeof(argv[0]));
				ReportErrorAndExit("VoIP parser",
				    strcat(strcpy(tail, "Unknown option "), argv[0]),
				    programName, flowId);
				break;
			}
		} else {
			char temp[sizeof("What is  ?") + sizeof(argv[h])];
			ReportErrorAndExit("VoIP parser",
			    strcat(strcat(strcpy(temp, "What is "), argv[0]), " ?"),
			    programName, flowId);
		}
	}
	delete(*pPktSize);
	PktSizeDistro = pdConstant;
	ConstantRV = new Constant(1);
	*pPktSize = new SumRandom((VAD * framesize * samples) * (*ConstantRV) + header);
	delete(*pIntArriv);
	IntArrivDistro = pdConstant;
	ConstantRV = new Constant(1);
	*pIntArriv = new SumRandom(1000.0 / pkts * (*ConstantRV));
	printf
	    ("Voice Codec: %s\t\nFramesize: %4.2f\t\nSamples: %4.0f\t\nPackets per sec.: %4.0f\t\n",
	    codec, framesize, samples, pkts);
	if (VAD == 1)
		printf("VAD: No\n");
	else
		printf("VAD: Si\n");
}

void dnsParser(SumRandom ** pIntArriv, SumRandom ** pPktSize, TDistro & IntArrivDistro,
    TDistro & PktSizeDistro)
{
	
	delete(*pIntArriv);
	IntArrivDistro = pdConstant;
	ConstantRV = new Constant(1);
	*pIntArriv = new SumRandom(1000.0 / 0.56 * (*ConstantRV));

	
	delete(*pPktSize);
	PktSizeDistro = pdUniform;
	UniformRV = new Uniform;
	*pPktSize = new SumRandom(220.0 * (*UniformRV) + 100.0);
}




void CSParsera(SumRandom ** pIntArriv, SumRandom ** pPktSize, TDistro & IntArrivDistro, TDistro & PktSizeDistro){

	delete(*pIntArriv);

	IntArrivDistro = pdStudent;
	StudentRV = new Student(41.6994, 0.104821, 0.704073);
	*pIntArriv = new SumRandom(1 * (*StudentRV));


	delete(*pPktSize);

	Real prob_PS[2];
	Random *pDis_PS[2];
	PktSizeDistro=pdCSa;

	Normal *normal_1=new Normal;
	SumRandom *normal_11=new SumRandom(34.8352 + (4.3915 * (*normal_1)));

	Normal *normal_2=new Normal;
	SumRandom *normal_22=new SumRandom(49.4904 + (3.6186 * (*normal_2)));


	prob_PS[0]=0.3732;
	prob_PS[1]=0.6268;

	pDis_PS[0]=(Random*)normal_11;
	pDis_PS[1]=(Random*)normal_22;


	MixedRandom *mix_rv=new MixedRandom(2,prob_PS,pDis_PS);


	*pPktSize = new SumRandom(1 * (*mix_rv));
}

void CSParseri(SumRandom ** pIntArriv, SumRandom ** pPktSize, TDistro &
IntArrivDistro, TDistro & PktSizeDistro){

	delete(*pIntArriv);

	IntArrivDistro = pdStudent;
	Student *Student_rv = new Student(41.6858, 0.0694393, 1.08341);
	*pIntArriv = new SumRandom(1 * (*Student_rv));


	delete(*pPktSize);


	PktSizeDistro=pdDiscrete;

	Real val[8];
	Real prob[8];

	val[0]=27;
	prob[0]=0.8312;

	val[1]=28;
	prob[1]=0.0018;

	val[2]=29;
	prob[2]=0.0453;

	val[3]=30;
	prob[3]=0.0195;

	val[4]=31;
	prob[4]=0.0879;

	val[5]=32;
	prob[5]=0.0024;

	val[6]=33;
	prob[6]=0.0106;

	val[7]=35;
	prob[7]=0.0013;

	static DiscreteGen P(8,prob,val);

	*pPktSize = new SumRandom(1 * P);
}


void QuakeParser(SumRandom ** pIntArriv, SumRandom ** pPktSize, TDistro & IntArrivDistro, TDistro & PktSizeDistro)
{

	delete (*pIntArriv);
	IntArrivDistro=pdQuake;
	Constant *constant_rv=new Constant(10.75);

	ExponentialRV= new Exponential(4.29);

	Real prob[2];

	prob[0]=0.6;
	prob[1]=0.4;

	Random *pDis_IAT[2]; 

	pDis_IAT[0]=(Random*)ExponentialRV;
	pDis_IAT[1]=(Random*)constant_rv;

	Mixed_rv=new MixedRandom(2,prob,pDis_IAT);

	*pIntArriv = new SumRandom(1 * (*Mixed_rv));


	delete (*pPktSize);
	PktSizeDistro=pdNormal;
	NormalRV=new Normal;
	*pPktSize=new SumRandom(3.2035755 * (*NormalRV) + 64.151757);

}


#ifdef BURSTY
void burstyParser(int &h, char *argv[], int &argc, unsigned int flowId, SumRandom ** OnPeriod,
	SumRandom ** OffPeriod, TDistro & OnPeriodDistro, TDistro & OffPeriodDistro)
{
	
	Real a,	b;
	switch (argv[h][0]) {
		case 'C': 
			if ((argc < 4) ||  (strtod(argv[h + 1],NULL) <= 0.))
				ReportErrorAndExit("Protocol Parser",
					"Invalid settings for the On/Off periods", programName, flowId);
			delete (*OnPeriod);
			OnPeriodDistro = pdConstant;
			ConstantRV = new Constant(1);
			*OnPeriod = new SumRandom(atoi(argv[h + 1]) * (*ConstantRV));
			h += 2;
			argc -= 2;
			break;
		case 'U':
   			if ((argc < 5) ||  (strtod(argv[h+1],NULL) <= 0.)
				||  (strtod(argv[h+2],NULL) <= strtod(argv[h+1],NULL)) )
				ReportErrorAndExit("Protocol Parser",
					"Invalid settings for the On/Off periods", programName, flowId);
			delete (*OnPeriod);
			OnPeriodDistro = pdUniform;
			a =  strtod(argv[h+2],NULL);
			b =  strtod(argv[h+1],NULL) - a ;
			UniformRV = new Uniform;
			*OnPeriod = new SumRandom( b * (*UniformRV) +  a);
			h += 3;
			argc -= 3;
			break;								
		case 'E':
			if ((argc < 4) || (strtod(argv[h+1],NULL) <= 0.))
				ReportErrorAndExit("Protocol Parser",
					"Invalid settings for the On/Off periods", programName, flowId);
			delete (*OnPeriod);
			OnPeriodDistro = pdExponential;
			ExponentialRV = new Exponential;
			*OnPeriod =
			new SumRandom(strtod(argv[h+1],NULL) * (*ExponentialRV));
			h += 2;
			argc -= 2;
			break;
		case 'V':
			if ((argc < 5) || (strtod(argv[h + 1], NULL) <= 0)
				|| (strtod(argv[h + 2], NULL) <= 0))
				ReportErrorAndExit("Protocol Parser",
					"Invalid settings for the On/Off periods", programName, flowId);
			a = (Real) strtod(argv[h + 1], NULL);
			b = (Real) strtod(argv[h + 2], NULL);
			ParetoRV = new Pareto(a);
			delete (*OnPeriod);
			OnPeriodDistro = pdPareto;
			*OnPeriod = new SumRandom(b * (*ParetoRV));
			h += 3;
			argc -= 3;
			break;
		case 'Y':
			if ((argc < 5) || (strtod(argv[h + 2], NULL) <= 0))	
				ReportErrorAndExit("Protocol Parser",
					"Invalid settings for the On/Off periods", programName, flowId);
			b = strtod(argv[h + 1], NULL); 
			a = strtod(argv[h + 2], NULL); 
			CauchyRV = new Cauchy;
			delete (*OnPeriod);
			OnPeriodDistro = pdCauchy;
			*OnPeriod = new SumRandom(a * (*CauchyRV) + b);
			h += 3;
			argc -= 3;
			break;
		case 'N':
			if ((argc < 5) || (argv[h + 2] <= 0))	
				ReportErrorAndExit("Protocol Parser",
					"Invalid settings for the On/Off periods", programName, flowId);
			b = strtod(argv[h + 1], NULL); 	
			a = strtod(argv[h + 2], NULL); 	
			delete (*OnPeriod);
			OnPeriodDistro = pdNormal;
			NormalRV = new Normal;
			*OnPeriod = new SumRandom(a * (*NormalRV) + b);
			h += 3;
			argc -= 3;
			break;
		case 'O':
			if ((argc < 4) || (strtod(argv[h + 1], NULL) <= 0))	
				ReportErrorAndExit("Protocol Parser",
					"Invalid settings for the On/Off periods", programName, flowId);
			a = strtod(argv[h + 1], NULL);
			delete (*OnPeriod);
			
			PoissonRV = new Poisson(a);
			OnPeriodDistro = pdPoisson;
			*OnPeriod = new SumRandom( 1 * (*PoissonRV));
			h += 2;
			argc -= 2;
			break;
		case 'G':
			if ((argc < 5) || (strtod(argv[h + 1], NULL) <= 0) || (strtod(argv[h + 2], NULL) <= 0))	
				ReportErrorAndExit("Protocol Parser",
					"Invalid settings for the On/Off periods", programName, flowId);
			a = (Real) strtod(argv[h + 1], NULL);
			b = (Real) strtod(argv[h + 2], NULL);
    			delete (*OnPeriod);
			
			GammaRV = new Gamma(a);
			OnPeriodDistro = pdGamma;
			*OnPeriod = new SumRandom(b * (*GammaRV));
			h += 3;
			argc -= 3;
			break;
		case 'W':
			   if ((argc < 5) || (strtod(argv[h + 1], NULL) <= 0) ||
			       (strtod(argv[h + 2], NULL) <= 0))
			     ReportErrorAndExit("Protocol Parser",
					"Invalid settings for the On/Off periods", programName, flowId);
			   a = (Real) strtod(argv[h+1],NULL);
			   b = (Real) strtod(argv[h+2],NULL);
			   delete (*OnPeriod);
			   WeibullRV = new Weibull(a,b);
			   OnPeriodDistro = pdWeibull;
			   *OnPeriod = new SumRandom( 1 * (*WeibullRV));
			   h += 3;
			   argc -= 3;
			   break;
		default:
			ReportErrorAndExit("Protocol Parser",
				"Invalid settings for the On/Off periods", programName, flowId);
			break;
		}

	
	switch (argv[h][0]) {
		case 'C': 
			if ((argc < 2) ||  (strtod(argv[h + 1],NULL) <= 0.))
				ReportErrorAndExit("Protocol Parser",
					"Invalid settings for the Off/Off periods", programName, flowId);
			delete (*OffPeriod);
			OffPeriodDistro = pdConstant;
			ConstantRV = new Constant(1);
			*OffPeriod = new SumRandom(atoi(argv[h + 1]) * (*ConstantRV));
			h += 2;
			argc -= 2;
			break;
		case 'U':
   			if ((argc < 3) ||  (strtod(argv[h+1],NULL) <= 0.)
				||  (strtod(argv[h+2],NULL) <= strtod(argv[h+1],NULL)) )
				ReportErrorAndExit("Protocol Parser",
					"Invalid settings for the Off/Off periods", programName, flowId);
			delete (*OffPeriod);
			OffPeriodDistro = pdUniform;
			a =  strtod(argv[h+2],NULL);
			b =  strtod(argv[h+1],NULL) - a ;
			UniformRV = new Uniform;
			*OffPeriod = new SumRandom( b * (*UniformRV) +  a);
			h += 3;
			argc -= 3;
			break;								
		case 'E':
			if ((argc < 2) || (strtod(argv[h+1],NULL) <= 0.))
				ReportErrorAndExit("Protocol Parser",
					"Invalid settings for the Off/Off periods", programName, flowId);
			delete (*OffPeriod);
			OffPeriodDistro = pdExponential;
			ExponentialRV = new Exponential;
			*OffPeriod =
			new SumRandom(strtod(argv[h+1],NULL) * (*ExponentialRV));
			h += 2;
			argc -= 2;
			break;
		case 'V':
			if ((argc < 3) || (strtod(argv[h + 1], NULL) <= 0)
				|| (strtod(argv[h + 2], NULL) <= 0))
				ReportErrorAndExit("Protocol Parser",
					"Invalid settings for the Off/Off periods", programName, flowId);
			a = (Real) strtod(argv[h + 1], NULL);
			b = (Real) strtod(argv[h + 2], NULL);
			ParetoRV = new Pareto(a);
			delete (*OffPeriod);
			OffPeriodDistro = pdPareto;
			*OffPeriod = new SumRandom(b * (*ParetoRV));
			h += 3;
			argc -= 3;
			break;
		case 'Y':
			if ((argc < 3) || (strtod(argv[h + 2], NULL) <= 0))	
				ReportErrorAndExit("Protocol Parser",
					"Invalid settings for the Off/Off periods", programName, flowId);
			b = strtod(argv[h + 1], NULL); 
			a = strtod(argv[h + 2], NULL); 
			CauchyRV = new Cauchy;
			delete (*OffPeriod);
			OffPeriodDistro = pdCauchy;
			*OffPeriod = new SumRandom(a * (*CauchyRV) + b);
			h += 3;
			argc -= 3;
			break;
		case 'N':
			if ((argc < 3) || (argv[h + 2] <= 0))	
				ReportErrorAndExit("Protocol Parser",
					"Invalid settings for the Off/Off periods", programName, flowId);
			b = strtod(argv[h + 1], NULL); 	
			a = strtod(argv[h + 2], NULL); 	
			delete (*OffPeriod);
			OffPeriodDistro = pdNormal;
			NormalRV = new Normal;
			*OffPeriod = new SumRandom(a * (*NormalRV) + b);
			h += 3;
			argc -= 3;
			break;
		case 'O':
			if ((argc < 2) || (strtod(argv[h + 1], NULL) <= 0))	
				ReportErrorAndExit("Protocol Parser",
					"Invalid settings for the Off/Off periods", programName, flowId);
			a = strtod(argv[h + 1], NULL);
			delete (*OffPeriod);
			
			PoissonRV = new Poisson(a);
			OffPeriodDistro = pdPoisson;
			*OffPeriod = new SumRandom( 1 * (*PoissonRV));
			h += 2;
			argc -= 2;
			break;
		case 'G':
			if ((argc < 3) || (strtod(argv[h + 1], NULL) <= 0) || (strtod(argv[h + 2], NULL) <= 0))	
				ReportErrorAndExit("Protocol Parser",
					"Invalid settings for the Off/Off periods", programName, flowId);
			a = (Real) strtod(argv[h + 1], NULL);
			b = (Real) strtod(argv[h + 2], NULL);
    			delete (*OffPeriod);
			
			GammaRV = new Gamma(a);
			OffPeriodDistro = pdGamma;
			*OffPeriod = new SumRandom(b * (*GammaRV));
			h += 3;
			argc -= 3;
			break;
		case 'W':
			   if ((argc < 3) || (strtod(argv[h + 1], NULL) <= 0) ||
			       (strtod(argv[h + 2], NULL) <= 0))
			     ReportErrorAndExit("Protocol Parser",
					"Invalid settings for the On/Off periods", programName, flowId);
			   a = (Real) strtod(argv[h+1],NULL);
			   b = (Real) strtod(argv[h+2],NULL);
			   delete (*OffPeriod);
			   WeibullRV = new Weibull(a,b);
			   OffPeriodDistro = pdWeibull;
			   *OffPeriod = new SumRandom( 1 * (*WeibullRV));
			   h += 3;
			   argc -= 3;
			   break;
		default:
			ReportErrorAndExit("Protocol Parser",
				"Invalid settings for the On/Off periods", programName, flowId);
			break;
		}

	
}
#endif

