/* Originally Dillon Buchanan's Prog1, modified by Tyler Carroll */

#include <iostream>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <string>
#include <list>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdarg.h>

//The port to operate on
#define PORT 10310

//The backlog for listening
#define BACKLOG 5

//The deliminator for splitting incomming commands
#define DELIMINATOR " "

//The buffer length for receiving commands
#define BUFFER_LEN 1024

//A character that indicates the end of a response.
#define END_CHAR "\x1a"

//Using standard namespace
using namespace std;

//A database entry
struct db_entry
{
    int id_number;
    string first_name;
    string last_name;
    string location;

    //Print out the database structure
    string print() const
    {
        stringstream ss;
        ss << setfill('0') << setw(9) << id_number << ": " << first_name << " "
           << last_name << " at " << location;
        return ss.str();
    }
};

//Our database of entries
static list<db_entry> database;

//The number of clients services during the server's operation
static int clientsServiced = 0;

//Mutex to control database access - TC
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

//Verbose Print Global Variable
static bool verbose = false;

//Dump the database to a file
void db_dump()
{
    //Open a file to dump the database.
    ofstream myfile("LDatabase.txt");

    //Make sure we successfully opened the file.
    if (!myfile)
    {
        cerr << "Unable to open LDatabase.txt for database dump!" << endl;
        return;
    }
   
	//Get mutex lock
	pthread_mutex_lock(&mutex1);

    //Dump all the database entries
    for (list<db_entry>::const_iterator it = database.begin(); it != database.end(); it++)
        myfile << it->print() << endl;
   
	//Get mutex lock
	pthread_mutex_unlock(&mutex1);
}

//Check if entry exists!
const db_entry * get_entry(int id)
{
	const db_entry *ret = NULL;
   
	//Get mutex lock
	pthread_mutex_lock(&mutex1);

    for (list<db_entry>::const_iterator it = database.begin(); it != database.end(); it++){
        if (it->id_number == id){
            ret = &(*it);
			break;
		}
	}
   
	//Get mutex lock
	pthread_mutex_unlock(&mutex1);
	
    return ret;
}

//A compare function for the database.
bool compare_db_entry(const db_entry & first, const db_entry & second)
{
	return strcasecmp(first.last_name.c_str(), second.last_name.c_str()) <= 0;
}

//Adding an entry to the database
bool add_entry(const int id, const string & first, 
               const string & last, const string & location)
{
    //Get the entry to make sure it doesnt exist already.
    if (get_entry(id))
        return false;
 
    //Create a database entry
    db_entry entry = { id, first, last, location };
   
	//Get mutex lock
	pthread_mutex_lock(&mutex1);
    
    //Insert into the database
    database.push_back(entry);

    //Do sorting 
    database.sort(&compare_db_entry);
   
	//Get mutex lock
	pthread_mutex_unlock(&mutex1);

    //Added the entry
    return true;
}

//Get's the integer
bool getInt(const string & str, int & val)
{
    //Loop through all of the characters to make sure they are numeric
    for (int j = 0; j < str.length(); j++)
    {
        if (!isdigit(str[j]))
        return false;
    }

    //Convert from string to an interger and check for conversion errors.
    stringstream i(str);
    return (i >> val);
}

//Is the string only alpha characters
bool isAlpha(const string & str)
{
    for (int j = 0; j < str.length(); j++)
    {
        if (!isalpha(str[j]))
            return false;
    }
    return true;
}

//Removes an entry
bool remove_entry(const int id, db_entry * ret)
{
	bool bool_ret = false;
 
	//Get mutex lock
	pthread_mutex_lock(&mutex1);

    for (list<db_entry>::iterator it = database.begin(); it != database.end(); it++)
    {
        if (it->id_number == id)
        {
            if (ret) *ret = *it;
            database.erase(it);
            bool_ret =  true;
			break;
        }
    }
   
	//Get mutex lock
	pthread_mutex_unlock(&mutex1);
    
	return bool_ret;
}


/* Error with errno set */
void error(const char * str)
{
    perror(str);
    exit(1);
}

/* 
    Respond to the 
*/
void respond(const int sock, const string & buffer)
{
    if (write(sock, buffer.c_str(), buffer.length()) != buffer.length())
        error("Unable to send data!");
}


// Handles a client connection
bool handleclient(const int socket)
{ 
    //Something to make sending a complex string a little easier
    #define SEND(a) do { stringstream _ss; _ss << a << "\n" << END_CHAR; \
                         respond(socket, _ss.str()); } while(0)
    #define SENDMORE(a) do { stringstream _ss; _ss << a << "\n"; respond(socket, _ss.str()); } while(0)

    int issued = 0;
    string myName;
    while (true)
    {
        //Shall we get some data?
        char buffer[BUFFER_LEN + 1] = {0}; //Allow for null.
        int len = read(socket, buffer, BUFFER_LEN);

        //Some sort of error
        if (len < 0)
        {
            cerr << "Unable to read from client socket!" << endl;
            break;
        }

        //Disconnected
        if (len == 0)
        {
            cerr << "The client disconnected!" << endl;
            break;
        }

        //Tokenize our string
        const char * cmd = strtok(buffer, DELIMINATOR);
        
        //The client send an invalid command!
        if (cmd == NULL)
            continue;

        //Client sent a command
        ++issued;
        
        //Did they send the login command?
        if (strcmp(cmd, "login") == 0)
        {
            //Get the name
            char * name = strtok(NULL, DELIMINATOR);

            if (!name)
            {
                SEND("Invalid argument: login <name>");
                continue;
            }

            if (strlen(name) > 10)
            {
                SEND("The name cannot be longer than 10 characters!");
                continue;
            }

            //Save our name
            myName = name;

            //Send the response
            SEND("Hello " << name << "!");
        }
        else if (strcmp(cmd, "add") == 0)
        {
            //Get information from the command line
            char * id       = strtok(NULL, DELIMINATOR);
            char * first    = strtok(NULL, DELIMINATOR);
            char * last     = strtok(NULL, DELIMINATOR);
            char * location = strtok(NULL, DELIMINATOR);

            if (!id || !first || !last || !location)
            {
                SEND("Invalid argument: add <id> <first> <last> <location>");
                continue;
            }

            if (strlen(id) != 9)
            {
                SEND("ID must be exactly 9 digits!");
                continue;
            }
            
            int iId = 0;
            if (!getInt(id, iId))
            {
                SEND("Invalid id!");
                continue;
            }

            if (strlen(first) > 20)
            {
                SEND("First name cannot be more than 20 characters!");
                continue;
            }

            if (strlen(last) > 25)
            {
                SEND("Last name cannot be more than 25 characters!");
                continue;
            }

            if (strlen(location) > 30)
            {
                SEND("Location cannot be more than 30 characters!");
                continue;
            }

            if (!isAlpha(first))
            {
                SEND("First name must be alpha characters!");
                continue;
            }

            if (!isAlpha(last))
            {
                SEND("Last name must be alpha characters!");
                continue;
            }

            //Add the entry
            if (add_entry(iId, first, last, location))
                SEND("Successfully inserted: " << id << ": " << 
                             first << " " << last << " at " << location);
            else
                SEND("Unable to add entry: " << id << " already exists!");
        }
        else if (strcmp(cmd, "remove") == 0)
        {
            char * id = strtok(NULL, DELIMINATOR);

            if (!id)
            {
                SEND("Invalid argument: remove <id>");
                continue;
            }

            if (strlen(id) != 9)
            {
                SEND("ID must be exactly 9 digits!");
                continue;
            }
            
            int iId = 0;
            if (!getInt(id, iId))
            {
                SEND("Invalid id!");
                continue;
            }
            
            //Was not able to insert item
            db_entry ent;
            if (remove_entry(iId, &ent))
                SEND("Removed entry: " << ent.first_name << " " << ent.last_name);
            else
                SEND("Unable to remove entry: " << id << "!");
        }
        else if (strcmp(cmd, "list") == 0)
        {
            char * start  = strtok(NULL, DELIMINATOR);
            char * finish = strtok(NULL, DELIMINATOR);
            int sent = 0;

			//Get mutex lock
			pthread_mutex_lock(&mutex2);
            
			if (database.size() == 0)
            {
                SEND("No database entries!");
                continue;
            }
			
			//Get mutex lock
			pthread_mutex_unlock(&mutex2);

            if (start != NULL)
            {
                if (strlen(start) > 2)
                {
                    SEND("Start can only be 1 or 2 characters!");
                    continue;
                }

                if (!isAlpha(start))
                {
                    SEND("Start must be alpha characters only!");
                    continue;
                }
            }
            
            if ((start != NULL) && (strlen(start) == 1))
            {
                if (finish != NULL)
                {
                    SEND("If start only has 1 character last is not needed!");
                    continue;
                }

				//Get mutex lock
				pthread_mutex_lock(&mutex2);

                for (list<db_entry>::const_iterator it = database.begin(); it != database.end(); it++)
                {
                    if (strncasecmp(start, it->last_name.c_str(), 1) == 0)
                    {
                        SENDMORE(it->print());
                        ++sent;
                    }
                }

				//Get mutex lock
				pthread_mutex_unlock(&mutex2);
            }
            else if (start != NULL && finish == NULL)
            {
				//Get mutex lock
				pthread_mutex_lock(&mutex2);

                for (list<db_entry>::const_iterator it = database.begin(); it != database.end(); it++)
                {
                    if (strncasecmp(it->last_name.c_str(), start, strlen(start)) == 0)
                    {
                        SENDMORE(it->print());
                        ++sent;
                    }
                }

				//Get mutex lock
				pthread_mutex_unlock(&mutex2);
            }
            else if (start != NULL && finish != NULL)
            {

                if (strlen(finish) != 2)
                {
                    SEND("Finish can only have 2 characters!");
                    continue;
                }

                if (!isAlpha(finish))
                {
                    SEND("Finish must have alpha characters only!");
                    continue;
                }

				//Get mutex lock
				pthread_mutex_lock(&mutex2);

                for (list<db_entry>::const_iterator it = database.begin(); it != database.end(); it++)
                {
                    if (strncasecmp(it->last_name.c_str(), start, strlen(start)) >= 0 &&
                        strncasecmp(it->last_name.c_str(), finish, strlen(finish)) <= 0)
                    {
                        SENDMORE(it->print());
                        ++sent;
                    }
                }

				//Get mutex lock
				pthread_mutex_unlock(&mutex2);
            }
            else
            {
				//Get mutex lock
				pthread_mutex_lock(&mutex2);
             
			   for (list<db_entry>::const_iterator it = database.begin(); it != database.end(); it++)
                {
                    SENDMORE(it->print());
                    ++sent;
                }

				//Get mutex lock
				pthread_mutex_unlock(&mutex2);
            }
            
            // Check if any data was sent.
            if (sent == 0)
            {
                SEND("no entries!");
            }
            else
            {
                //Send the end character so the client knows it's all over.
                respond(socket, END_CHAR);
            }
        }
        else if (strcmp(cmd, "quit") == 0)
        {
            //Grab the argument
            char * eof = strtok(NULL, DELIMINATOR);

            //The end of the file is processed
            if ((eof != NULL) && (strcmp(eof, "EOF") == 0))
            {
                //Send a EOF
                SEND("Goodbye, " << myName << ". You sent " << issued << " commands!" <<
                     endl << "I'm leaving. I proccessed " << clientsServiced << " clients");

                //NO MORE CLIENTS!
                return false;
            }
            else if (eof != NULL)
            {
                SEND("Incorrect argument: quit [EOF]");
                continue;
            }
            else
            {
                //Send the goodbye
                SEND("Goodbye, " << myName << ". You sent " << issued << " commands!");
            }

            //We can accept more clients
            break;
        }
        else
        {
            //No such command
            SEND("What are you talking about?");
        }
    }

    return true;
}

//functino to handle new client as a new thread - TC
void *handleclient_thread(void *newsock){

	int *sock = (int*)(newsock);
	
	handleclient(*sock);

	close(*sock);
}

//Verbose printing mode - TC
void printv(char *format, ...){

	if(verbose == false)
		return;

	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

//Main function!
int main(char argc, char *argv[])
{
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen = sizeof(cli_addr);
	pthread_t thread;

	//Check if we are in verbose mode - TC
	if(argc == 2){
		if(strcmp(argv[1], "-v") == 0){
			verbose = true;
			printv("Verbose Print Mode Enabled!\n");
		}
	}else{
		verbose = false;
	}

    //Create the sock handle
	printv("Creating socket\n");
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        error("Unable to create sock!");
    
    //Clear the structures just incase
    memset(&cli_addr,  0, sizeof(cli_addr));
    memset(&serv_addr, 0, sizeof(serv_addr));
    
    //Fill in the server structure
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);
    
    //Bind the socket to a local address
	printv("Binding socket\n");
    if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("Unable to bind sock!");

    //Begin listening on that port
    if (listen(sock, BACKLOG) < 0)
        error("Unable to listen on sock!");

    bool soldierOn = true;
    while (soldierOn)
    {
        //Accept a new connection!
		printv("Listening for connections...\n");
        int newsock = accept(sock, (struct sockaddr *)&cli_addr, &clilen);
        if (newsock < 0)
            error("Unable to accept connection!");

        //We're handling another client
        ++clientsServiced;

        //Start a new thread to handle clients - TC
		printv("Connection made! Starting new thread...\n");
		if(pthread_create(&thread, NULL, handleclient_thread, (void*)&newsock) < 0)
			error("Unable to create new thread!\n");

    }

    //Dump the database to a file
    db_dump();
    
    //All done.
    return 0;
}
