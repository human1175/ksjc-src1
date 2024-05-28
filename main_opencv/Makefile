CC = g++
CFLAGS = -Wall -I/usr/local/include/opencv4
LDFLAGS = -L/usr/local/lib -lwiringPi -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_objdetect -lopencv_videoio

SRC_C = main.c tracking_sensor.c yb_pcb_car.c
SRC_CPP = qr_recognition.cpp
OBJ_C = $(SRC_C:.c=.o)
OBJ_CPP = $(SRC_CPP:.cpp=.o)
OBJ = $(OBJ_C) $(OBJ_CPP)

TARGET = robot

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
