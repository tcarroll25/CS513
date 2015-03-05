Tyler Carroll
CS513
Program 1

In the main directory are the following files:
Binaries:
	client
	server

C++ Files:
	tcc_client.cpp
	tcc_server.cpp

Input Files:
	LClient.txt

Output Files:
	LClient.log
	LDatabase.txt

Makefiles:
	Makefile

Makefile Commands:
	make - make both binaries
	make clean - delete both binaries AND both output files

Run the following commands in two different PuTTy windows:

Window 1:
	./server

Window 2:
	./client "CCCWORK#.WPI.EDU" ["LClient.txt"]

LClient.txt is an optional command that will run the commands in the .txt file, if that command is omitted then the client will allow the user to call commands from the command line.
