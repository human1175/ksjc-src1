CC = gcc
CFLAGS = -Wall -lwiringPi -I.
DEPS = yb_pcb_car.h tracking_sensor.h
OBJ_MOTOR = main_motor.o yb_pcb_car.o
OBJ_TRACKING_SENSOR = main_tracking_sensor.o tracking_sensor.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# motor_test: $(OBJ_MOTOR)
# 	$(CC) -o $@ $^ $(CFLAGS)

tracking_sensor_test: $(OBJ_TRACKING_SENSOR)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o motor_test tracking_sensor_test
