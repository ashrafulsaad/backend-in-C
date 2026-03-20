CC = gcc
CFLAGS = -Wall -pthread

all: server

server: server.c test.c server.h
	$(CC) $(CFLAGS) server.c test.c -o server

clean:
	rm -f server
