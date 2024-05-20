CC = g++
CFLAGS = -I/usr/local/include/opencv4
LDFLAGS = -L/usr/local/lib -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_videoio -lopencv_objdetect
TARGET = qrrecognition

all: $(TARGET)

$(TARGET): qr_recognition.o
	$(CC) -o $(TARGET) qr_recognition.o $(LDFLAGS)

qr_recognition.o: qr_recognition.cpp
	$(CC) -c qr_recognition.cpp $(CFLAGS)

clean:
	rm -f *.o $(TARGET)
