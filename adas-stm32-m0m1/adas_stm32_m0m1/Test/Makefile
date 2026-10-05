# Host unit tests (run on the PC, no hardware):  make -C Test
CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wconversion -Wshadow -Werror -g -O0 \
         -I../Lib -I../Config -I../Components/encoder -I../Components/motor_driver \
         -I../BSP -I./hoststub

all: test_encoder test_motor
	./test_encoder
	./test_motor

test_encoder: test_encoder.c ../Components/encoder/encoder.c
	$(CC) $(CFLAGS) -o $@ $^

test_motor: test_motor.c stub_bsp.c ../Components/motor_driver/motor_driver.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f test_encoder test_motor
