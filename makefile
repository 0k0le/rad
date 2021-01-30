#
## rad - makefile
## Matthew Todd Geiger
## 1/13/2021
#

CC=gcc

rad: rad.o
	$(CC) -o rad rad.o

rad.o: rad.c
	$(CC) -Wall -Wextra -O2 -c rad.c 

clean:
	rm -f rad.o rad

install:
	rm -f /usr/bin/rad
	cp rad /usr/bin/rad
