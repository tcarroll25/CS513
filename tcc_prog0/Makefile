basic:
	gcc tcc_echo_client.c -c -o echo_client.o
	gcc tcc_echo_server.c -c -o echo_server.o
	gcc tcc_socket.c -c -o socket.o
	gcc echo_client.o socket.o -o echo_client
	gcc echo_server.o socket.o -o echo_server 
	rm -rf *.o

enhanced:
	gcc tcc_enhanced_echo_client.c -o enhanced_echo_client
	gcc tcc_enhanced_echo_server.c -o enhanced_echo_server

clean:
	rm -rf echo_client echo_server enhanced_echo_client enhanced_echo_server *.o
