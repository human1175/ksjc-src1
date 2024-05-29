all: Server Client

Server: server.c server.h
	gcc -Wall -g server.c -o Server -lpthread

Client: client.c server.h
	gcc -Wall -g client.c -o Client
