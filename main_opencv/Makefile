CC=gcc
CXX=g++
CFLAGS=-Wall -O2
CXXFLAGS=-Wall -O2 `pkg-config --cflags opencv4`
LDFLAGS=`pkg-config --libs opencv4` -lpthread -lwiringPi

OBJS=main.o yb_pcb_car.o tracking_sensor.o client.o qr_recognition.o

all: robot

robot: $(OBJS)
	$(CXX) $(CFLAGS) -o robot $(OBJS) $(LDFLAGS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

yb_pcb_car.o: yb_pcb_car.c
	$(CC) $(CFLAGS) -c yb_pcb_car.c

tracking_sensor.o: tracking_sensor.c
	$(CC) $(CFLAGS) -c tracking_sensor.c

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

qr_recognition.o: qr_recognition.cpp
	$(CXX) $(CXXFLAGS) -c qr_recognition.cpp

clean:
	rm -f *.o robot
