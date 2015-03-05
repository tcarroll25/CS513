/* Tyler Carroll - prog0 - this code was adapted from tcp sockets for C */

#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h> /* for sockaddr_in and inet_addr() */
#include <stdlib.h> /* for atoi() */
#include <string.h> /* for memset() */
#include <unistd.h> /* for close() */
#include <time.h>

#define RCVBUFSIZE 128

void DieWithError(char *errorMessage)
{
	perror(errorMessage);
	exit(1);
}

int main(int argc, char *argv[])
{
	int sock; /* Socket descriptor */
	struct sockaddr_in echoServAddr; /* Echo server address */
	unsigned short echoServPort;
	char *servlP;
	int i;

	char echoBuffer[RCVBUFSIZE];
	unsigned int echoStringLen[6];
	unsigned int totalStringLen = 0;
	int bytesRcvd, totalBytesRcvd;
	
	if (argc != 7) /* Test for correct number of arguments */
	{
		fprintf(stderr, "Usage: ip_addr \"str1\" \"str2\" \"str3\" \"str4\" \"str5\"\n");
		exit(1);
	}

	servlP = argv[1];
	
	for(i = 0; i < 5; i++){	
		echoStringLen[i] = strlen(argv[i+2]);
		totalStringLen += strlen(argv[i+2]);
	}
	/* append end transmission packet */
	echoStringLen[5] = strlen("DLE ETX");
	totalStringLen = strlen("DLE ETX");	

	echoServPort = 10001; /* 7 is the well-known port for the echo service */
	
	/* Create a reliable, stream socket using TCP */
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		DieWithError(" socket () failed") ;
	
	/* Construct the server address structure */
	memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
	echoServAddr.sin_family = AF_INET; /* Internet address family */
	echoServAddr.sin_addr.s_addr = inet_addr(servlP); /* Server IP address */
	echoServAddr.sin_port = htons(echoServPort); /* Server port */
	
	/* Establish the connection to the echo server */
	if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
		DieWithError(" connect () failed") ;
	
	/* Send the string to the server */
	for(i = 0; i < 5; i++){
		if (send(sock, (char*)argv[i+2], echoStringLen[i], 0) != echoStringLen[i])
			DieWithError("send() sent a different number of bytes than expected");
		usleep(1000);
	}
 	if (send(sock, (char*)"DLE ETX", echoStringLen[5], 0) != echoStringLen[5])
                        DieWithError("send() sent a different number of bytes than expected");
	
	/* Receive the same string back from the server */
	totalBytesRcvd = 0;
	printf("Received: "); /* Setup to print the echoed string */
	while (totalBytesRcvd < totalStringLen)
	{
		/* Receive up to the buffer size (minus 1 to leave space for
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
