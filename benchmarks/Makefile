CC = gcc
CFLAGS = -g -m32 -w -Wall -Wvla -Werror

all: clean test

test: 
	$(CC) $(CFLAGS) -o test test.c -L../ -lmy_vm
clean:
	rm -rf test