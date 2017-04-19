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




#ifdef UNIX
#include <sys/wait.h>
#endif
#ifdef WIN32
#include <time.h>
#include <math.h>
#endif

#include "ITG.h"
#include "timestamp.h"
#include <cmath>

#ifdef WIN32

LARGE_INTEGER freq;
#endif



void setSeedRandom()
{
#ifdef UNIX
		
		struct timeval tv;
		
		gettimeofday(&tv, NULL);
		
		srand(tv.tv_usec);
#endif
#ifdef WIN32
		
		SYSTEMTIME lpSystemTime;
		
		GetSystemTime(&lpSystemTime);
		
		srand(lpSystemTime.wSecond);
#endif

}

#ifdef WIN32
void updateTicker(struct TTicker *Ticker, LARGE_INTEGER & _tend, LARGE_INTEGER & _tprec, LARGE_INTEGER & _tstart, int &first)
{
	if (first) {
		_tprec = _tstart;
		first = 0;
	}
	QueryPerformanceCounter(&_tend);
	Ticker->count += (((((double) _tend.QuadPart - (double) _tprec.QuadPart)) / (double) freq.QuadPart) * 1e3);
	_tprec = _tend;
}


int tstart(LARGE_INTEGER & _tstart, unsigned long &secs, unsigned long &msecs, int &first, BYTE meter, int flag)
{
	
	SYSTEMTIME lpSystemTime;
	
	BOOL ret;
	
	if (first) {

		
		ret = QueryPerformanceFrequency(&freq);
		if (ret == 0)
			return -1;
		first = 0;
	}
	
    GetSystemTime(&lpSystemTime);
	ret = QueryPerformanceCounter(&_tstart);
	if (ret == 0)
		return -1;

	

	
	secs = lpSystemTime.wHour * 3600 + lpSystemTime.wMinute * 60 + lpSystemTime.wSecond;

	PRINTD(3,"tstart: GetSytemTime.second %lu \n",(unsigned long)secs);

    
	msecs = lpSystemTime.wMilliseconds;

	if ((meter == METER_OWDM) || (flag == RECEIVER)) {
		
	msecs = lpSystemTime.wMilliseconds;
	} else {
		
	msecs = lpSystemTime.wMilliseconds * 1000;
	}

   return 0;
}

int gettimeofday(struct timeval *thisTime, LARGE_INTEGER & _tend, LARGE_INTEGER & _tstart,
    unsigned long &secs, unsigned long &msecs, BYTE meter, int flag)
{
	
	double secAndUsec = 0;
	
	long micro = 0;
	
	BOOL ret;
	
	ret = QueryPerformanceCounter(&_tend);
	if (ret == 0)
		return -1;
	 
	secAndUsec = ((((double) _tend.QuadPart - (double) _tstart.QuadPart)) / (double) freq.QuadPart);
    	PRINTD(3,"gettimeofday: QueryPerformanceCounter:secAndUsec %lf \n", secAndUsec);
	
	thisTime->tv_sec = ((unsigned long) secAndUsec) + secs;
	
	if ((meter == METER_OWDM) || (flag == RECEIVER)) {
		
		micro = (long) ceil(((secAndUsec - floor(secAndUsec)) * 1e6) / 1000);
		
		if ((thisTime->tv_usec = (micro + msecs)) >= 1e3) {
			
			thisTime->tv_sec++;
			
			thisTime->tv_usec = (long int)(((double) (thisTime->tv_usec * 1e-3) - 1) * 1e3);
		}
		
		thisTime->tv_usec = (long int)(thisTime->tv_usec * 1e3);
	} else {
		
		micro = (long) ((secAndUsec - floor(secAndUsec)) * 1e6);
		
		if ((thisTime->tv_usec = (micro + msecs)) >= 1e6) {
			
			thisTime->tv_sec++;
			
			thisTime->tv_usec = (long int)(((double) (thisTime->tv_usec * 1e-6) - 1) * 1e6);
		}
	}
	return 0;
}
#endif

#ifdef UNIX
void updateTicker(struct TTicker *Ticker)
{
	struct timeval thisTime, *lastTime = &Ticker->lastTime;

	gettimeofday(&thisTime, NULL);
	Ticker->count += ((double) (thisTime.tv_sec - lastTime->tv_sec)) * 1000.0 +
	    ((double) (thisTime.tv_usec - lastTime->tv_usec)) / 1000.0;
	*lastTime = thisTime;

}
#endif
