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



#include "ITG.h"

#ifdef UNIX
struct sched_param p;
#endif


const char *meters[] = { "", "OWDM", "RTTM", " " };
const char *l4Protocols[] = { "", "TCP", "UDP", "ICMP", "SCTP", "DCCP", " " };
const char *l7Protocols[] = { "", "Telnet", "VoIP", "DNS", "AoM", "CSa", "CSi", "Quake3", " " };
char programName[100];		
char DEFAULT_LOG_IP[10] = "127.0.0.1";	






extern const int StandardMinPayloadSize = 4 * sizeof(uint32_t); 
extern const int ShortMinPayloadSize = 2 * sizeof(uint32_t); 
extern const int NoneMinPayloadSize = sizeof(uint32_t);	



USHORT checksum(USHORT * buffer, int size)
{
	unsigned long cksum = 0;

	while (size > 1) {
		cksum += *buffer++;
		size -= sizeof(USHORT);
	}

	if (size) {
		cksum += *(UCHAR *) buffer;
	}

	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);
	return (USHORT) (~cksum);
}

#ifdef WIN32
int InitializeWinsock(WORD wVersionRequested)
{
	WSADATA wsaData;
	int err;

	err = WSAStartup(wVersionRequested, &wsaData);

	
	
	if (err != 0)
		return 0;	

	
	
	
	

	
	
	
	
	

	
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
		return 0;

	
	return 1;
}




void sleep(int tempo)
{
	Sleep(tempo * 1000);
}
#endif





char *putValue(void *startPos, void *data, int size)
{
	
	char *endPos;
	
	memcpy(startPos, data, size);
	
	endPos = (char *) startPos + size;
	return endPos;
}

BYTE findMeter(char *s)
{
	int i = 1;

	while (strcmp(meters[i], " "))
		if (!strcasecmp(s, (char *) meters[i]))
			return(i);
		else
			i++;
	return LX_ERROR_BYTE;
}

BYTE findL4Proto(char *s)
{
	int i = 1;
	while (strcmp(l4Protocols[i], " "))
		if (!strcasecmp(s, (char *) l4Protocols[i]))
			return(i);
		else
			i++;
	return LX_ERROR_BYTE;
}

BYTE findL7Proto(char *s)
{
	int i = 1;
	while (strcmp(l7Protocols[i], " "))
		if (!strcasecmp(s, (char *) l7Protocols[i]))
			return(i);
		else
			i++;
	return LX_ERROR_BYTE;
}

const char* invFindMeter(BYTE meter)
{
	return meters[meter];
}

const char* invFindL4Proto(BYTE proto)
{
	return l4Protocols[proto];
}

const char* invFindL7Proto(BYTE proto)
{
	return l7Protocols[proto];
}


#ifdef WIN32
char
*strtok_r (char *restrict string, const char *restrict delim, char **saveptr)
{
    size_t i = 0;
    size_t j;
    int found_token;
    int found_delim;
    
    
    
    
    
    if (string == NULL)
      {
        if (*saveptr == NULL)
          {
            return NULL;
          }
        else
          {
            string = *saveptr;
          }
      }
    
    
    
    
    if (string[0] == 0)
      {
        *saveptr = NULL;
        return NULL;
      }
    else
      {
        if (delim[0] == 0)
          {
            return string;
          }
      }
    
    
    
    for (i = 0, found_token = 0, j = 0; string[i] != 0 && (!found_token); i++)
      {
        
        
        
        for (j = 0, found_delim = 0; delim[j] != 0; j++)
          {
            if (string[i] == delim[j])
              {
                found_delim = 1;
              }
          }
        
        
        
        
        
        if (!found_delim)
          {
            found_token = 1;
            break;
          }
      }
    
    
    
    
    
    
    if (found_token)
      {
        string += i;
      }
    else
      {
        *saveptr = NULL;
        return NULL;
      }
    
    
    
    for (i = 0, found_delim = 0; string[i] != 0; i++)
      {
        for (j = 0; delim[j] != 0; j++)
          {
            if (string[i] == delim[j])
              {
                found_delim = 1;
                break;
              }
          }
        if (found_delim)
          {
            break;
          }
      }
    
    
    
    
    
    
    
    if (found_delim)
      {
        string[i] = 0;
        *saveptr = &string[i+1];
      }
    else
      {
        *saveptr = NULL;
      }
    
    
    
    
    return string;
}

#endif

void printVersion(const char* ProgramName)
{
	
	printf("%s", ProgramName);

#ifdef VERSION
	
	printf(" version %s", VERSION);
#endif

#ifdef REVISION
	
	printf(" (r%s)", REVISION);
#endif
	printf("\n");

	
	printf("Compile-time options:");

#ifdef SCTP
	printf(" sctp");
#endif
#ifdef DCCP
	printf(" dccp");
#endif
#ifdef BURSTY
	printf(" bursty");
#endif
#ifdef MULTIPORT
	printf(" multiport");
#endif
#if defined WIN32 && not defined IPv6RECV
	printf(" ipv4only");
#endif
#ifdef DEBUG
	printf(" debug");
#endif
	printf("\n");

}

