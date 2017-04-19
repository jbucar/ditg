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



#ifdef CRYPTO
#include <openssl/rsa.h>
#include <openssl/pem.h>

void generate_key()
{
	RSA *RSAreceiver;
	FILE *keyReceiver1;
	FILE *keyReceiver2;
	keyReceiver1 = fopen("KeyReceiverPubblica", "w");
	keyReceiver2 = fopen("KeyReceiverPrivata", "w");
	RSAreceiver = RSA_generate_key(1024, 17, 0, 0);
	
	PEM_write_RSAPublicKey(keyReceiver1, RSAreceiver);
	PEM_write_RSAPrivateKey(keyReceiver2, RSAreceiver, 0, 0, 0, 0, 0);
	fclose(keyReceiver1);
	fclose(keyReceiver2);

	RSA *RSAsender;
	FILE *KeySender1;
	FILE *KeySender2;
	KeySender1 = fopen("KeySenderPubblica", "w");
	KeySender2 = fopen("KeySenderPrivata", "w");
	RSAsender = RSA_generate_key(1024, 17, 0, 0);
	
	PEM_write_RSAPublicKey(KeySender1, RSAsender);
	PEM_write_RSAPrivateKey(KeySender2, RSAsender, 0, 0, 0, 0, 0);
	
	fclose(KeySender1);
	fclose(KeySender2);
}

int autenticazioneSender(int signaling)
{
	char *next;
	int size;
	BYTE type = 10;
	unsigned char Messaggio[256];
	unsigned char MessaggioCodificato[256];
	unsigned char MessaggioCodificato2[256];
	unsigned char MessaggioOriginale[] = "fantozzi alla riscossa";
	unsigned char MessaggioOriginale2[256];
	unsigned char buffer[3000];
	generate_key();
	int plen = sizeof(MessaggioOriginale) - 1;
	RSA *RSAreceiver;
	FILE *keyReceiver;
	keyReceiver = fopen("KeyReceiverPubblica", "r");
	PEM_read_RSAPublicKey(keyReceiver, &RSAreceiver, 0, 0);
	fclose(keyReceiver);

	
	size = RSA_public_encrypt(plen, MessaggioOriginale, MessaggioCodificato, RSAreceiver, RSA_PKCS1_PADDING);	
	cout << "Esito della RSA_pubblic_encrypt :" << size << endl;

	
	next = putValue(&Messaggio, (void *) &type, sizeof(type));
	next = putValue(next, (void *) &MessaggioCodificato, sizeof(MessaggioCodificato));
	size = send(signaling, (char *) &Messaggio, 129, 0);


	size = recv(signaling, (char *) &type, sizeof(type), 0);
	size = recv(signaling, (char *) &MessaggioCodificato2, sizeof(MessaggioCodificato2), 0);
	

	RSA *RSAsender;
	FILE *keysender;
	keysender = fopen("KeySenderPrivata", "r");
	PEM_read_RSAPrivateKey(keysender, &RSAsender, 0, 0);
	fclose(keysender);
	size =
	    RSA_private_decrypt(size, MessaggioCodificato2, MessaggioOriginale2, RSAsender,
	    RSA_PKCS1_PADDING);
	cout << "Esito della RSA_private decrypt :" << size << endl;

	
	
	if (strcmp((const char *) MessaggioOriginale, (const char *) MessaggioOriginale2) == 0)
		return 0;
	else
		return 1;
}

int autenticazioneReceiver(int signaling){
	int size;
        char *next;
	BYTE type=10;
	unsigned char Messaggio[256];
	unsigned char MessaggioCodificato2[256];
	unsigned char MessaggioCodificato[256];
	unsigned char MessaggioDecodificato[256];
	unsigned char MessaggioOriginale[256];
	unsigned char buffer[1000];

	size=recv(signaling,(char*)&MessaggioCodificato,sizeof(MessaggioCodificato),0);
 	
	

	RSA*RSAreceiver;
	FILE *keyreceiver;
	keyreceiver=fopen("KeyReceiverPrivata","r");
	PEM_read_RSAPrivateKey(keyreceiver,&RSAreceiver,0,0);
	fclose(keyreceiver);
	size=RSA_private_decrypt( size, MessaggioCodificato, MessaggioOriginale, RSAreceiver,  RSA_PKCS1_PADDING);
	
	

	RSA*RSAsender;
	FILE *keysender;
	keysender=fopen("KeySenderPubblica","r");
	PEM_read_RSAPublicKey(keysender,&RSAsender,0,0);
	fclose(keysender);
	size=RSA_public_encrypt( size, MessaggioOriginale, MessaggioCodificato2, RSAsender,RSA_PKCS1_PADDING);  		

	

	next=putValue(&Messaggio,(void*)&type,sizeof(type));
  	next=putValue(next,(void*)&MessaggioCodificato2,sizeof(MessaggioCodificato2));
	size=send(signaling,(char*)&Messaggio,129,0);

	
}

#endif
