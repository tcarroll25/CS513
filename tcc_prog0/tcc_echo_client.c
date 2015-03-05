/* Tyler Carroll - Program 0 - This code is mostly from TCP Sockets for C */

#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h> /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tcc_socket.h"

#define RCVBUFSIZE 32

int main(int argc, char *argv[])
{
	int sock; /* Socket descriptor */
	struct sockaddr_in echoServAddr; /* Echo server address */
	unsigned short echoServPort;
	char *servIP;
	char *echoString;
	char echoBuffer[RCVBUFSIZE];
	unsigned int echoStringLen;
	int bytesRcvd, totalBytesRcvd;

	if ((argc< 3) || (argc> 4)) /* Test for correct number of arguments */
	{
	fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n",
		argv[0]);
		exit(1);
	}

	servIP = argv[1] ;
	echoString = argv[2] ;
	
	/* First arg' server IP address (dotted quad) */
	/* Second arg' string to echo */
	if (argc == 4)
		echoServPort = atoi(argv[3]); /* Use given port, if any */
	else
		echoServPort = 7; /* 7 is the well-known port for the echo service */

	sock = tcc_connect(servIP, echoServPort);

	tcc_send(echoString, sock);

	echoStringLen = strlen(echoString);
	
	/* Receive the same string back from the server */
	totalBytesRcvd = 0;
	printf("Received: "); /* Setup to print the echoed string */
	while (totalBytesRcvd < echoStringLen)
	{
		/* Receive up to the buffer size (minus i to leave space for
		a null terminator) bytes from the sender */
		if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
		DieWithError("recv() failed or connection closed prematurely");
		totalBytesRcvd += bytesRcvd; /* Keep tally of total bytes */
		echoBuffer[bytesRcvd] = '\0'; /* Terminate the string! */
		printf(echoBuffer); /* Print the echo buffer */
	}
	printf("\n"); /* Print a final linefeed */

	close(sock);
	exit(0);
}
