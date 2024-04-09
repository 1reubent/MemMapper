CC = gcc
CFLAGS = -g -m32 -w -Wall -Wvla -Werror

all: clean test mat_test

test: 
	$(CC) $(CFLAGS) -o $@ test.c -L../ -lmy_vm
mat_test:
	$(CC) $(CFLAGS) -o $@ mat_test.c -L../ -lmy_vm
clean:
	rm -rf test mat_test