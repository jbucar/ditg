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


#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>

#ifdef DEBUG
	#define PRINTD(level, ...)		if (DEBUG >= level) { printf(__VA_ARGS__); fflush(stdout); }
#else
	#define PRINTD(level, ...)
#endif

#ifdef DEBUG
	#ifdef UNIX
		#define PRINTDS(level, ...)	    if (DEBUG >= level) { usleep(1000); printf(__VA_ARGS__); fflush(stdout); }
	#endif
	#ifdef WIN32
		#define PRINTDS(level, ...)	    if (DEBUG >= level) { Sleep(1000); printf(__VA_ARGS__); fflush(stdout); }
	#endif
#else
	#define PRINTDS(level, ...)
#endif

#endif

