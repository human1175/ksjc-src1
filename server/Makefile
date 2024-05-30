all: ClientTest

ClientTest: main.c client.c client.h server.h
	gcc -Wall -g main.c client.c -o ClientTest

clean:
	rm -f ClientTest
