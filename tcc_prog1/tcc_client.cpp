/* Tyler Carroll - Program 1 - tcc_client.cpp */
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <list>
#include <sstream>
#include <fstream>
#include <errno.h>

#define SERVER_PORT 10310
#define BUFFER 2048
#define ENDCHAR '\xAA'

using namespace std;

int main(int argc, char *argv[])
{
	int sd, len;
	struct hostent *hp;
	struct sockaddr_in server;
	ifstream commands;
	string command, tmp, user;
	ofstream log("LClient.log");
	bool login = false;
	char buffer[BUFFER+1] = {0};

	/* Check for the correct number of arguements */
	if ((argc< 2) || (argc> 3)){
		fprintf(stderr, "Usage: %s <Server Machine Name> [<File.txt>]\n",
			argv[0]);
			exit(1);
	}
	
	/* Open file if it is passed */
	if(argc == 3){
		commands.open(argv[2]);
		if(commands == NULL){
			perror(strerror(errno));
			exit(-1);
		}
	}
	
	/* Make sure log is ready */
	if(log == NULL){
		perror(strerror(errno));
		exit(-1);
	}
	
	/* Prepare Socket */
	if((sd = socket(AF_INET,SOCK_STREAM,0)) < 0){
		perror(strerror(errno));
		exit(-1);
	}
	
	/* Send/Recieve commands */
	while(1){
		memset(buffer, 0, BUFFER+1);
		
		/* Either read from command file or get command from stdin */
		if(commands){
			getline(commands, command);
		}else{
			cout << "Enter Command: ";
			cout.flush();
			getline(cin, command);
		}
		
		/* Get the first word and see what command it is */
		stringstream temp(command);

		if(!(temp >> tmp)){
			continue;
		}else if(!( (tmp == "login")||(tmp == "add")||(tmp == "remove")||(tmp == "list")||(tmp == "quit") )){
			cout << "Valid commands are: login add remove list quit" << endl;
			continue;
		}else if(tmp == "login"){
			/* Checked to see if we are already logged in */
			if(login){
				cout << "You are already logged in as: " << user << endl;
				cout << "You must log out before logging in as a different user" << endl;
				continue;
			}

			/* check login name, if invalid go to next command */
			if(!(temp >> user)){
				cout << "You forgot a login name" << endl;
				continue;
			}else if(user.length() > 10){
				cout << user << " is not a valid name" << endl;
				continue;
			}else{
				cout << user << " is now logged in!" << endl;
			}

			/* Establish connection */
		
			/* Prepare Socket */
			if((sd = socket(AF_INET,SOCK_STREAM,0)) < 0){
				perror(strerror(errno));
				exit(-1);
			}

			/* Get server address from name */
			bzero((char*)&server, sizeof(server));
			server.sin_family = AF_INET;
			server.sin_port = htons(SERVER_PORT);
			if((hp = gethostbyname(argv[1])) == NULL){
				perror(strerror(errno));
				exit(-1);
			}
			bcopy(hp->h_addr, (char*)&server.sin_addr, hp->h_length);

			/* Connect to server */
			if(connect(sd, (struct sockaddr*)&server, sizeof(server))<0){
				perror(strerror(errno));
				exit(-1);
			}
			
			cout << user << " is successfully connected!" << endl;

			/* set login variable to true */
			login = true;
		}

		if(login){

			/* Send command to server */
			cout << "Sending command to server!" << endl;
			len = write(sd, command.c_str(), command.length());
			if(len != command.length()){
				cout << "Unable to send the command!" << endl;
			}
			/* Recieve response from server */
			while(1){
				len = read(sd, buffer, BUFFER);
				
				if(len < 0){
					cout << "Cannot read anything from socket" << endl;
					perror(strerror(errno));
					exit(-1);
				}else if(len == 0){
					cout << "Disconnected from server" << endl;
					perror(strerror(errno));
					exit(-1);
				}
				/* print output to stdout after buffer is complete and escape char overwritten */
				/* check for EOF */
				if(buffer[len-1] == ENDCHAR){
					/* replace with NULL character */
					buffer[len-1] = '\x00';
					cout << buffer;
					log << buffer;
					break;
				}else{
					cout << buffer;
					log << buffer;
				}
			}
			/* Check is quit command was issued */
			if(tmp == "quit"){
				cout << "Logging out from user " << user << endl;
				login = false;
				close(sd);
				
				/* Check if EOF and break out of loop if present */
				temp >> tmp;
				if(tmp == "EOF"){
					cout << "EOF Reached, exiting program" << endl;
					break;
				}
			}	

		}else{
			cout << "You need to login first before issuing commands" << endl;
		}
	}

	close(sd);
	exit(0);
}
