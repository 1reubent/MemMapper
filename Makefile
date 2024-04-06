CC = gcc
CFLAGS = -g -c
AR = ar -rc
RANLIB = ranlib

all: clean  my_vm.a

my_vm.a: my_vm.o
	$(AR) libmy_vm.a my_vm.o
	$(RANLIB) libmy_vm.a

my_vm.o: my_vm.h
	$(CC) $(CFLAGS) -m32 -o my_vm.o my_vm.c

#test: 
#	$(CC) -g -m32 -w -Wall -Wvla -Werror -o test test.c my_vm.a

clean:
	rm -rf *.o *.a
