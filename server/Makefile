all: Server QRClient

Server: server.c
	gcc -Wall -g server.c -o Server -lpthread

QRClient: main.c client.c client.h qr_recognition.cpp qr_recognition.h
	g++ -Wall -g main.c client.c qr_recognition.cpp -o QRClient `pkg-config --cflags --libs opencv4`
