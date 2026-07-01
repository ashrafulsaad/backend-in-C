CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -I. -Iinclude -pthread
LDFLAGS = -pthread
SOURCES = src/core/server.c src/http/http.c src/router/router.c src/middleware/middleware.c src/threadpool/threadpool.c src/net/socket.c

all:
	$(CC) $(CFLAGS) examples/basic_server.c $(SOURCES) -o server $(LDFLAGS)

parser-test:
	$(CC) $(CFLAGS) tests/test_http_parser.c $(SOURCES) -o tests/test_http_parser $(LDFLAGS)
	./tests/test_http_parser

run: all
	./server

clean:
	rm -f server tests/test_http_parser