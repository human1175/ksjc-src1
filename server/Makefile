all: Server Client QRClient

Server: server.c server.h
	gcc -Wall -g server.c -o Server -lpthread

Client: client.c server.h
	gcc -Wall -g client.c -o Client

QRClient: main.c qr_recognition.cpp client.c server.h
	g++ -Wall -g main.c qr_recognition.cpp client.c -o QRClient `pkg-config --cflags --libs opencv4`
