CC = gcc
CFLAGS = -Wall -lwiringPi -I.
LDFLAGS = -lm
DEPS = yb_pcb_car.h tracking_sensor.h
OBJ = main.o yb_pcb_car.o tracking_sensor.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

linetracer: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

.PHONY: clean

clean:
	rm -f *.o linetracer
