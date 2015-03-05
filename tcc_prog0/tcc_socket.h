/* Tyler Carroll - tcc_socket.h - tcp/ip library */

#define MAXPENDING 5

void DieWithError(char *errorMessage);
int tcc_listen(unsigned short echoServPort);
int tcc_connect(char *servIP, unsigned short echoServPort);
