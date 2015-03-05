/* Tyler Carroll - Program 1 - tcc_server.cpp */
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
#include <iomanip>

#define SERVER_PORT 10310
#define BUFFER 2048
#define ENDCHAR '\xAA'
#define DELIM " "
/*#define BACKLOG SOMAXCONN*/
#define BACKLOG 5
#define ENTIRE_LIST 1
#define SINGLE_LETTER 2
#define DOUBLE_LETTERS 3
#define REMOVE_ENTRIES_BETWEEN_USERS 0

using namespace std;

/* global varibles */
static unsigned int totalClients = 0;
static unsigned int totalCommands = 0;

/* database list struct definitions */

struct entry{
	unsigned int id;
	string first;
	string last;
	string location;

	string print() const{
		stringstream out("");
		out << "ID: " << setw(9) << setfill('0') <<  id << " Name: " << first << " " << last << " Location: " << location;
		return out.str();
	}
};

/* database list */
static list<entry> database;

/* list function prototypes */
void dumpDatabase();
bool entryExist(int idd);
bool compareEntry(const entry & first, const entry & second);
bool addEntry(const int id, const string & first, const string & last, const string & location);
bool removeEntry(const int idd, entry * out);
void removeAllEntry();
int makeInt(const string & str);

int main(int argc, char *argv[])
{
	int sd, nsd, len, i, ID;
	struct sockaddr_in server;
	struct sockaddr_in client;
	char buffer[BUFFER+1] = {0};
	string command, tmp, user, totalCommandStr, totalClientStr;	
	char *id, *first, *last, *location;
	char *start, *finish;
	int start_len, finish_len;	
	stringstream out, response, temp;
	entry delEntry;
	int list_type = 0;
	list<entry>::const_iterator iter;

	sd = socket(AF_INET, SOCK_STREAM, 0);
	if(sd < 0){
		cout << "Unable to create socket" << endl;
		perror(strerror(errno));
		exit(-1);
	}

	bzero((char*)&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sd,(struct sockaddr*)&server, sizeof(server)) < 0){
		cout << "Unable to bind to socket" << endl;
		perror(strerror(errno));
		exit(-1);
	}

	if(listen(sd, BACKLOG) < 0){
		cout << "Unable to listen on socket" << endl;
		perror(strerror(errno));
		exit(-1);	
	}	

	unsigned int cltsize = sizeof(client);

	while(1){
		/* accept socket */
		nsd = accept(sd, (struct sockaddr*)&client, &cltsize);
		/*increase number of clients */
		totalClients++;
		/*clear out database*/
		if(REMOVE_ENTRIES_BETWEEN_USERS){
			removeAllEntry();
		}

		while(1){
			/* clear buffer */
			memset(buffer, 0, BUFFER+1);
			response.clear();

			/* read/write */
			cout << "Waiting for packets..." << endl;
			len = read(nsd, buffer, BUFFER);
			cout << "Packets recieved!" << endl;
			
			/*increase number of total commands recieved*/
			totalCommands++;

			if(len < 0){
				cout << "Cannot read anything from socket" << endl;
				perror(strerror(errno));
				exit(-1);
			}else if(len == 0){
				cout << "Disconnected from server" << endl;
				perror(strerror(errno));
				exit(-1);
			}

			cout << "buffer: " << buffer << endl;
			
			/* get command */
			command = strtok(buffer, DELIM);

			if(command == "login"){
				
				/* get user */
				user = strtok(NULL, DELIM);
				/* form response */
				response.str("");
				response << "Hello " << user << "!" << "\n" << ENDCHAR;
				/* write response */
				len = write(nsd, response.str().c_str(), response.str().length());
				cout << "Recieved login, sending back: " << response.str() << endl;

				/*check if we sent command*/
				if(len != response.str().length()){
					cout << "Unable to send the command!" << endl;
				}

			}else if(command == "add"){
	
				/* get database info */
				id = NULL;
				first = NULL;
				last = NULL;
				location = NULL;

				id = strtok(NULL, DELIM);
				first = strtok(NULL, DELIM);
				last = strtok(NULL, DELIM);
				location = strtok(NULL, DELIM);

				/* clear response string */
				response.str("");

				/*error checking */
				if( (first==NULL) || (last==NULL) || (id==NULL) || (location == NULL) ){
					response << "ADD ERROR: Correct syntax is: add id first last location\n" << ENDCHAR;
					goto ADD_SEND;
				}
				/* ID must be 9 digits */
				if(strlen(id) != 9){
					response << "ADD ERROR: ID must be nine digits long!\n" << ENDCHAR;
					goto ADD_SEND;
				}

				/* convert to number */
				for(i = 0; i < strlen(id); i++){
					if(isdigit(id[i]) == 0){
						response << "ADD ERROR: ID must contain only numberic values!\n" << ENDCHAR;
						goto ADD_SEND;
					}
				}

				ID = makeInt(id);

				/* check to make sure first name is alpha only */
				for(i = 0; i < strlen(first); i++){
					if(isalpha(first[i]) == 0){
						response << "ADD ERROR: First name must contain only alpha values!\n" << ENDCHAR;
						goto ADD_SEND;
					}
				}
				/* check to make sure last name is alpha only */
				for(i = 0; i < strlen(last); i++){
					if(isalpha(last[i]) == 0){
						response << "ADD ERROR: Last name must contain only alpha values!\n" << ENDCHAR;
						goto ADD_SEND;
					}
				}
				/* first name cannot be more than 20 */
				if(strlen(first) > 20){
					response << "ADD ERROR: First name can be a maximum of 20 characters!\n" << ENDCHAR;
					goto ADD_SEND;
				}

				/* last name cannot be more than 25 */
				if(strlen(last) > 25){
					response << "ADD ERROR: Last name can be a maximum of 25 characters!\n" << ENDCHAR;
					goto ADD_SEND;
				}

				/* location cannot be more than 30 */
				if(strlen(location) > 30){
					response << "ADD ERROR: Location can be a maximum of 30 characters!\n" << ENDCHAR;
					goto ADD_SEND;
				}
				
				/* add to database */
				if(addEntry(ID, first, last, location)){
					response << setw(9) << setfill('0') << ID << " " << first << " " << last << " " << location <<
						" was added to the database!\n" << ENDCHAR;
				}else{
					response << "ADD ERROR: ID: " << setw(9) << setfill('0') << ID << " already exists!\n" << ENDCHAR;
				}

ADD_SEND:				/* write response */
				len = write(nsd, response.str().c_str(), response.str().length());

				cout << "Sent back: " << response.str() << endl;
				
				/*check if we sent command*/
				if(len != response.str().length()){
					cout << "Unable to send the command!" << endl;
				}

			}else if(command == "remove"){
			
				id = NULL;

				/* get database info */
				id = strtok(NULL, DELIM);

				/* clear response string */
				response.str("");

				/*error checking */
				if(id==NULL){
					response << "REMOVE ERROR: Correct syntax is: remove id\n" << ENDCHAR;
					goto REMOVE_SEND;
				}
				/* ID must be 9 digits */
				if(strlen(id) != 9){
					response << "REMOVE ERROR: ID must be nine digits long!\n" << ENDCHAR;
					goto REMOVE_SEND;
				}

				/* convert to number */
				for(i = 0; i < strlen(id); i++){
					if(isdigit(id[i]) == 0){
						response << "REMOVE ERROR: ID must contain only numberic values!\n" << ENDCHAR;
						goto REMOVE_SEND;
					}
				}
				
				ID = makeInt(id);
				
				/* remove from database, send back name */
				if(removeEntry(ID, &delEntry)){
					response << delEntry.first << " " << delEntry.last << " was removed from the database!\n" << ENDCHAR;
				}else{
					response << "REMOVE ERROR: ID: " << setw(9) << setfill('0') << ID << " does not exist!\n" << ENDCHAR;
				}

REMOVE_SEND:			/* write response */
				len = write(nsd, response.str().c_str(), response.str().length());

				cout << "Sent back: " << response.str() << endl;
				
				/*check if we sent command*/
				if(len != response.str().length()){
					cout << "Unable to send the command!" << endl;
				}
			}else if(command == "list"){

				/* get database info */
				start = NULL;
				finish = NULL;

				start = strtok(NULL, DELIM);
				finish = strtok(NULL, DELIM);

				/* clear response string */
				response.str("");

				/*error checking */
				/* make sure there are entries to list */
				if(database.size() == 0){
					response << "LIST ERROR: There are no database entries to list!\n" << ENDCHAR;
					goto LIST_SEND_ERROR;
				}

				/* if no start, send whole list */
				if(start == NULL){
					/* make start = A, end = Z? */
					list_type = ENTIRE_LIST;
					goto LIST_SEND;
				}

				start_len = strlen(start);

				/* start length can only be 0, 1 or 2 */				
				if(start_len > 2){
					response << "LIST ERROR: Start must be one or two characters!\n" << ENDCHAR;
					goto LIST_SEND_ERROR;
				}

				/* check to make sure start is alpha only */
				for(i = 0; i < start_len; i++){
					if(isalpha(start[i]) == 0){
						response << "LIST ERROR: Start must contain only alpha values!\n" << ENDCHAR;
						goto LIST_SEND_ERROR;
					}
				}

				/* if start length is one, there can be no finish */
				if(start_len == 1){
					if(finish != NULL){
						response << "LIST ERROR: Finish cannot be present if Start is only one character!\n" << ENDCHAR;
						goto LIST_SEND_ERROR;
					}
					list_type = SINGLE_LETTER;
					goto LIST_SEND;
				} 

				if(start_len == 2){
					/* check to make sure there is a finish */
					if(finish == NULL){
						response << "LIST ERROR: Finish must be present if start is two characters!\n" << ENDCHAR;
						goto LIST_SEND_ERROR;
					}
					/* check to make sure finish is 2 characters */
					finish_len = strlen(finish);
					if(finish_len != 2){
						response << "LIST ERROR: Finish must be two characters if start is two characters!\n" << ENDCHAR;
						goto LIST_SEND_ERROR;
					}
					/* check to make sure finish is alpha only */
					for(i = 0; i < finish_len; i++){
						if(isalpha(finish[i]) == 0){
							response << "LIST ERROR: Finish must contain only alpha values!\n" << ENDCHAR;
							goto LIST_SEND_ERROR;
						}
					}
					/* check to make sure start > finish */
					if(strncasecmp(start, finish, 2) > 0){
						response << "LIST ERROR: Start must be alpha numerically greater than finish!\n" << ENDCHAR;
						goto LIST_SEND_ERROR;
					}
					list_type = DOUBLE_LETTERS;
					goto LIST_SEND;
				}
				
LIST_SEND:
				switch(list_type){
					case ENTIRE_LIST:
						/* send entire list */
						response.str("");
						response << "Sending entire list:" << endl;
						cout << "Sent back: " << response.str();
						len = write(nsd, response.str().c_str(), response.str().length());	
						/*check if we sent command*/
						if(len != response.str().length()){
							cout << "Unable to send the command!" << endl;
						}						
						for(iter = database.begin(); iter != database.end(); iter++){
							
							response.str("");
							response << iter->print() << endl;
								
							len = write(nsd, response.str().c_str(), response.str().length());
							cout << "Sent back: " << response.str();
							
							/*check if we sent command*/
							if(len != response.str().length()){
								cout << "Unable to send the command!" << endl;
							}
						}


						break;

					case SINGLE_LETTER:
						/* iterate over list and only send entries that match the same first letter */
						response.str("");
						response << "Sending all entries that start with a lase name of " << start << endl;
						cout << "Sent back: " << response.str();
						len = write(nsd, response.str().c_str(), response.str().length());	
						/*check if we sent command*/
						if(len != response.str().length()){
							cout << "Unable to send the command!" << endl;
						}						
						for(iter = database.begin(); iter != database.end(); iter++){
							if(strncasecmp(start, iter->last.c_str(), 1) == 0){
								
								response.str("");
								response << iter->print() << endl;
									
								len = write(nsd, response.str().c_str(), response.str().length());
								cout << "Sent back: " << response.str();
								
								/*check if we sent command*/
								if(len != response.str().length()){
									cout << "Unable to send the command!" << endl;
								}
							}
						}
						break;

					case DOUBLE_LETTERS:					
						/* iterate over list and only send entries that are between first and last */
						response.str("");
						response << "Sending all entries that fall alphabetically between " << start << " and " << finish << endl;
						cout << "Sent back: " << response.str();
						len = write(nsd, response.str().c_str(), response.str().length());	
						/*check if we sent command*/
						if(len != response.str().length()){
							cout << "Unable to send the command!" << endl;
						}						
						for(iter = database.begin(); iter != database.end(); iter++){
							if( (strncasecmp(start, iter->last.c_str(), 2) <= 0) && (strncasecmp(finish, iter->last.c_str(), 2) >= 0) ){
								
								response.str("");
								response << iter->print() << endl;
									
								len = write(nsd, response.str().c_str(), response.str().length());
								cout << "Sent back: " << response.str();
							
								/*check if we sent command*/
								if(len != response.str().length()){
									cout << "Unable to send the command!" << endl;
								}
							}
						}
						break;
				}		
			
				/* send the done character */		
				response.str("");
				response << ENDCHAR;	
				len = write(nsd, response.str().c_str(), response.str().length());
				continue;
LIST_SEND_ERROR:	
				/* write response */
				len = write(nsd, response.str().c_str(), response.str().length());

				cout << "Sent back: " << response.str() << endl;
				
				/*check if we sent command*/
				if(len != response.str().length()){
					cout << "Unable to send the command!" << endl;
				}

			}else if(command == "quit"){

				/*send back response that connection will be closed for username
					if EOF is sent then send back number of clients processed
					and dump database to file */
					
				
						
				cout << "Quit recieved, disconnecting from " << user << endl;
				
				/* get string versions of total commands and clients, reset total commands*/
				out.str("");
				out << totalCommands;
				totalCommandStr = out.str();
				out.str("");
				out << totalClients;			
				totalClientStr = out.str();			
				totalCommands = 0;
				
				char *bye = strtok(NULL, DELIM);
	
				if((bye != NULL) && (strcmp(bye,"EOF") == 0)){
					cout << "EOF recieved, shutting down server" << endl;
					
					/* form response */
					response.str("");	
					response << "Connection will be close for " << user << 
					", they sent " << totalCommandStr << " commands. " << 
					totalClientStr << " clients have been processed in total." << 
					"\n" << ENDCHAR;
				
					/* write response */
					len = write(nsd, response.str().c_str(), response.str().length());

					cout << "Sent back: " << response.str() << endl;
					
					/*check if we sent command*/
					if(len != response.str().length()){
						cout << "Unable to send the command!" << endl;
					}
				
					goto DONE;
				}else{

					/* form response */
					response.str("");
					response << "Connection will be close for " << user << 
					", they sent " << totalCommandStr << " commands." << "\n" << ENDCHAR;
					
					/* write response */
					len = write(nsd, response.str().c_str(), response.str().length());

					cout << "Sent back: " << response.str() << endl;
					
					/*check if we sent command*/
					if(len != response.str().length()){
						cout << "Unable to send the command!" << endl;
					}

					/* break out of this command loop */
					break;
				}
			}

		}

		/* close socket */	
		close(nsd);
			
	}
DONE:
	/*dump database*/
	dumpDatabase();
	close(sd);
}

void dumpDatabase(){
	ofstream out("LDatabase.txt");
	list<entry>::iterator i;

	/* check to make sure files opened */
	if(!out){
		cout << "Could not open database" << endl;
		return;
	}

	/* print out each entry */
	for(i = database.begin(); i != database.end(); i++){
		out << i->print() << endl;
	}

	return;
}

bool entryExist(int idd){
	list<entry>::iterator i;
	
	for(i = database.begin(); i != database.end(); i++){
		if(i->id == idd){
			return true;
		}
	}

	return false;
}

bool compareEntry(const entry & first, const entry & second){
	return (strcasecmp(first.last.c_str(), second.last.c_str()) <= 0);
}

bool addEntry(const int id, const string & first, const string & last, const string & location){
	/* check to see if entry already exists with this ID */
	if(entryExist(id) == true){
		cout << "An entry with that ID already exists!" << endl;
		return false;
	}

	/* add new entry to list and sort */
	entry newEntry = {id, first, last, location};
	database.push_back(newEntry);

	/* sort database */
	database.sort(&compareEntry);
	return true;
}

bool removeEntry(const int idd, entry *out){
	list<entry>::iterator i;
	
	for(i = database.begin(); i != database.end(); i++){
		if(i->id == idd){
			(*out) = (*i);
			database.erase(i);
			return true;
		}
	}

	return false;
}

void removeAllEntry(){
	database.clear();
	return;
}

int makeInt(const string & str){
	stringstream num(str);
	int out;
	num >> out;
	return out;
}

