CC = g++
CFLAGS = -Wall -O2 -I/usr/local/include/opencv4
LDFLAGS = -L/usr/local/lib -lwiringPi -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_objdetect -lopencv_videoio -lopencv_dnn -lopencv_calib3d -lopencv_features2d -lopencv_flann -lopencv_photo -lopencv_video -lopencv_imgcodecs -lopencv_ml -lopencv_bgsegm -lopencv_ccalib -lopencv_face -lopencv_superres -lopencv_videostab -lopencv_dnn_superres -lopencv_aruco -pthread

SRC_C = main.c tracking_sensor.c yb_pcb_car.c client.c
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
