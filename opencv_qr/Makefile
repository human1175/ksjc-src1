CC = g++
CFLAGS = `pkg-config --cflags opencv4`
LDFLAGS = `pkg-config --libs opencv4`
TARGET = qrrecognition

all: $(TARGET)

$(TARGET): qr_recognition.o
	$(CC) -o $(TARGET) qr_recognition.o $(LDFLAGS)

qr_recognition.o: qr_recognition.cpp
	$(CC) -c qr_recognition.cpp $(CFLAGS)

clean:
	rm -f *.o $(TARGET)
