all: Server QRClient

Server: server.c server.h
	gcc -Wall -g server.c -o Server -lpthread

QRClient: main.c client.c client.h qr_recognition.cpp qr_recognition.h server.h
	g++ -Wall -g main.c client.c qr_recognition.cpp -o QRClient `pkg-config --cflags --libs opencv4`
