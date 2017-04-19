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




typedef enum { pdConstant, pdUniform, pdExponential, pdPareto, pdCauchy, pdNormal, pdPoisson, pdGamma, pdWeibull,
		pdTelnet, pdAoM, pdDiscrete, pdExtreme_Largest, pdQuake, pdStudent, pdCSa,pdCSi } TDistro;

extern const char *DistroStrings[];
extern const unsigned int DefaultPktPerSec;
extern const unsigned int DefaultPktSize;
extern const unsigned long DefaultDuration;
extern const unsigned long DefaultDelay;

extern Constant *ConstantRV;
extern Uniform *UniformRV;
extern Exponential *ExponentialRV;
extern Pareto *ParetoRV;
extern Cauchy *CauchyRV;
extern Normal *NormalRV;
extern Poisson *PoissonRV;
extern Gamma *GammaRV;

extern Weibull *WeibullRV;

extern Extreme_Largest *Extreme_LargestRV;
extern Student *StudentRV;



void telnetParser(SumRandom ** pIntArriv, SumRandom ** pPktSize, TDistro & IntArrivDistro,
    TDistro & PktSizeDistro);
void voIPParser(int h, char *argv[], int &argc, unsigned int flowId, SumRandom ** pIntArriv,
    SumRandom ** pPktSize, TDistro & IntArrivDistro, TDistro & PktSizeDistro);
void dnsParser(SumRandom ** pIntArriv, SumRandom ** pPktSize, TDistro & IntArrivDistro,
    TDistro & PktSizeDistro);

void CSParsera(SumRandom ** pIntArriv, SumRandom ** pPktSize, TDistro & IntArrivDistro,
    TDistro & PktSizeDistro);
void QuakeParser(SumRandom ** pIntArriv, SumRandom ** pPktSize, TDistro & IntArrivDistro, TDistro & PktSizeDistro);
void CSParseri(SumRandom ** pIntArriv, SumRandom ** pPktSize, TDistro & IntArrivDistro,
    TDistro & PktSizeDistro);

#ifdef BURSTY
void burstyParser(int &h, char *argv[], int &argc, unsigned int flowId, SumRandom ** OnPeriod,
	SumRandom ** OffPeriod, TDistro & OnPeriodDistro, TDistro & OffPeriodDistro);
#endif

