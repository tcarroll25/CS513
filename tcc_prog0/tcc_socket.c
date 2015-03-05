/* Tyler Carroll - tcc_socket.c - tcp/ip socket functions */

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tcc_socket.h"

void DieWithError(char *errorMessage)
{
        perror(errorMessage);
        exit(1);
}

int tcc_listen(unsigned short echoServPort)
{

	struct sockaddr_in echoServAddr;
	int servSock; 

	/* Create socket for incoming connections */
        if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
                DieWithError( "socket () failed") ;	

        /* Construct local address structure */
        memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
        echoServAddr.sin_family = AF_INET; /* Internet address family */
        echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
        echoServAddr.sin_port = htons(echoServPort); /* Local port */

        /* Bind to the local address */
        if (bind(servSock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
                DieWithError("bind () failed");

        /* Mark the socket so it will listen for incoming connections */
        if (listen(servSock, MAXPENDING) < 0)
                DieWithError("listen() failed");

	return servSock;
}

int tcc_connect(char* servIP, unsigned short echoServPort)
{
	struct sockaddr_in echoServAddr;
	unsigned int sock;
	

	/* Create a reliable, stream socket using TCP */
        if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
                DieWithError(" socket () failed") ;

        /* Construct the server address structure */
        memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
        echoServAddr.sin_family = AF_INET; /* Internet address family */
        echoServAddr.sin_addr.s_addr = inet_addr(servIP); /* Server IP address */
        echoServAddr.sin_port = htons(echoServPort); /* Server port */

        /* Establish the connection to the echo server */
        if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
                DieWithError(" connect () failed") ;

	return sock;
}

void tcc_send(char *echoString, int sock)
{
	unsigned int echoStringLen = strlen(echoString) ; /* Determine input length */

	/* Send the string to the server */
	if (send(sock, echoString, echoStringLen, 0) != echoStringLen)
		DieWithError("send() sent a different number of bytes than expected");
}
