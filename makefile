.c.o:
	gcc -g -c $?

# compile client and server
all: chatclient chatserver

# compile client only
chatclient: chatclient.o chatlinker.o
	gcc -g -o chatclient chatclient.o  chatlinker.o 

# compile server program
chatserver: chatserver.o chatlinker.o
	gcc -g -o chatserver chatserver.o  chatlinker.o 
