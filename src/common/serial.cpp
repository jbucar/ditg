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


#include "serial.h"

#ifdef WIN32
HANDLE serialUp(char *nameSerial)
{
	HANDLE ret = CreateFile(nameSerial,
                    GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING,
                    FILE_FLAG_OVERLAPPED, 0);

    return ret;
}



void DTR_Enable(HANDLE hComm)
{
	DCB dcb_app;

	GetCommState(hComm,&dcb_app);
	dcb_app.fDtrControl = DTR_CONTROL_ENABLE;
	SetCommState(hComm,&dcb_app);
}


void DTR_Disable(HANDLE hComm)
{
	DCB dcb_app;

	GetCommState(hComm,&dcb_app);
	dcb_app.fDtrControl = DTR_CONTROL_DISABLE;
	SetCommState(hComm,&dcb_app);
}

void RTS_Enable(HANDLE hComm)
{
	DCB dcb_app;

	GetCommState(hComm,&dcb_app);
	dcb_app.fRtsControl  = RTS_CONTROL_ENABLE;
	SetCommState(hComm,&dcb_app);
}

void RTS_Disable(HANDLE hComm)
{
	DCB dcb_app;

	GetCommState(hComm,&dcb_app);
	dcb_app.fRtsControl = RTS_CONTROL_DISABLE;
	SetCommState(hComm,&dcb_app);
}
#endif

#ifdef UNIX

#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>

HANDLE serialUp(char *nameSerial)
{
	int  fd;
	char stringa[]="/dev/";
	strcat(stringa, nameSerial);
	
	fd = open(stringa, O_RDWR | O_NOCTTY);
	
	return fd;
}

void DTR_Disable(HANDLE hComm)
{
	int status;
	ioctl(hComm, TIOCMGET, &status);
	status &= ~ TIOCM_DTR; 
	ioctl(hComm, TIOCMSET, &status);
}


void DTR_Enable(HANDLE hComm)
{
	int status;
	ioctl(hComm, TIOCMGET, &status);
	status |= TIOCM_DTR;  
	ioctl(hComm, TIOCMSET, &status);
}

void RTS_Disable(HANDLE hComm)
{
	int status;
	ioctl(hComm, TIOCMGET, &status);
	status &= ~ TIOCM_RTS; 
	ioctl(hComm, TIOCMSET, &status);
}

void RTS_Enable(HANDLE hComm)
{
	int status;
	ioctl(hComm, TIOCMGET, &status);
	status |= TIOCM_RTS;  
	ioctl(hComm, TIOCMSET, &status);
}

#endif
