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

//The buffer length
#define BUFFER_LEN 4096

//The port to connect to
#define PORT 10310

//Using standard namespace
using namespace std;

/* Error with errno set */
void error(const char * str)
{
    perror(str);
    exit(1);
}

/* Error without errno set */
void error2(const char * str)
{
    cerr << str << endl;
    exit(1);
}

//Converting the hostname to an IP address
int hostname_to_ip(const char * hostname , char* ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;

    if ( (he = gethostbyname( hostname ) ) == NULL)
        error2("Unable to get the hostname!");

    addr_list = (struct in_addr **) he->h_addr_list;

    for(i = 0; addr_list[i] != NULL; i++)
    {
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;
    }

    return 1;
}

//Main function
int main(char argc, char *argv[])
{
    int sock;
    struct sockaddr_in cli_addr;
    int clilen = sizeof(cli_addr);
    int readlen;
    char strip[64] = {0};

    //Make sure the arguments match
    if (argc > 3 || argc < 2)
    {
        fprintf(stderr, "Invalid usage: %s <target> [<file>]\n", argv[0]);
        return -1;
    }

    //Clear the structures just incase
    memset(&cli_addr,  0, sizeof(cli_addr));

    //Fill in the server structure
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_port = htons(PORT);
    
    if (hostname_to_ip(argv[1], strip) != 0)
        error2("Unable to determine IP from hostname!");

    //Create the sin_addr from the argument
    if (inet_pton(AF_INET, strip, &cli_addr.sin_addr) != 1)
        error2("Invalid IP address!");

    //Open a log file
    ofstream logfile("LClient.log");
    if (!logfile)
        error2("Unable top open LClient.log");

    //If there is a input file specified open that
    ifstream infile;
    if (argc == 3)
    {
        infile.open(argv[2]);
        if (!infile)
            error2("Unable to open file.");
    }

    bool loggedin = false;
    while (true)
    {
        string str, cmd;

        //If there is an input file specified grab from that
        if (infile)
        {
            getline(infile, str);
        }
        else
        {
            //Print out a command line
            cout << "-> "; 
            cout.flush();
            getline(cin, str); //Get a full line of text
        }

        //Grab the command portion
        stringstream strstr(str);
        if (!(strstr >> cmd))
            continue;

        //Is the user trying to login
		if (cmd == "register"){



		}else if (cmd == "login")
        {
            //Get the login name
            string name;
            if (!(strstr >> name) || name.length() > 20)
            {
                cout << "Invalid login command!" << endl;
                continue;
            }
            
            //Close it just incase.
            close(sock);

            //Create the sock handle (TCP Connection)
            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0)
                error("Unable to create sock!");

            //Connect to the target
            if (connect(sock, (struct sockaddr *)&cli_addr, sizeof(cli_addr)) < 0)
                error("Unable to connet to target address!");
            
            //We're now logged in!
            loggedin = true;
        }

        //If we're logged in we can receive data
        if (loggedin)
        {
            //Send the data
            int len = write(sock, str.c_str(), str.length());
            if (len != str.length())
                error("Unable to send the data!");

            //Keep receiving data until we get the end character
            bool done = false;
            while (!done)
            {
                //Get the response
                char buffer[BUFFER_LEN + 1] = {0};
                len = read(sock, buffer, BUFFER_LEN);

                //Some sort of error
                if (len < 0)
                    error("Unable to read from socket.");

                //Disconnected
                if (len == 0)
                    error2("Server disconnected!");

                //We received the end character!
                if (buffer[len - 1] == '\x1a')
                {
                    done = true;
                    buffer[len -1] = '\x00';
                }
                
                //Print the response
                cout << buffer;
                logfile << buffer;
            }

            //Stupid case
            if (cmd == "quit")
            {
                close(sock);
                loggedin = false;
                
                string arg;
                if ((strstr >> arg) && (arg == "EOF"))
                {
                    return 0;
                }
            }
        }
        else
        {
            cout << "You're not logged in!" << endl;
        }
    }

    return 0;
}

