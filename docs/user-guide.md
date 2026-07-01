# User Guide: Managing and Extending the C Backend Project

This project is a small but practical HTTP backend written in C from scratch. It is designed to help you learn how a real backend works at the system level: sockets, parsing, routing, middleware, concurrency, and simple response handling.

## 1. What this project does

The server can:
- listen for HTTP requests on a TCP port
- parse simple request lines and headers
- route requests to handlers
- apply middleware
- process requests with a small worker thread pool
- return demo responses for health, database status, benchmark, and echo

It is not a full production framework, but it is a strong learning foundation.

## 2. How to build and run

### Prerequisites
- Linux or WSL
- GCC
- Make
- curl

### Build
```bash
make
```

### Run the server
```bash
./server
```

The server listens on port 18080 by default for local demo use.

### Quick demo checks
```bash
curl http://127.0.0.1:18080/
curl http://127.0.0.1:18080/health
curl http://127.0.0.1:18080/db
curl -X POST http://127.0.0.1:18080/api/echo -d 'hello=world'
```

### Run the parser test
```bash
make parser-test
```

## 3. Project structure

```text
.
├── Dockerfile
├── Makefile
├── README.md
├── docs/
│   ├── architecture.md
│   └── user-guide.md
├── examples/
│   ├── basic_server.c
│   └── demo.sh
├── include/
│   └── server.h
├── src/
│   ├── core/
│   ├── http/
│   ├── middleware/
│   ├── net/
│   ├── router/
│   └── threadpool/
├── tests/
│   └── test_http_parser.c
└── server
```

## 4. Main code areas

### Core server
- [src/core/server.c](../src/core/server.c)
- Responsible for startup, route registration, middleware setup, database stub initialization, and request loop.

### HTTP layer
- [src/http/http.c](../src/http/http.c)
- Parses requests, extracts path/query/body, and prepares responses.

### Router
- [src/router/router.c](../src/router/router.c)
- Stores routes and dispatches requests to the correct handler.

### Middleware
- [src/middleware/middleware.c](../src/middleware/middleware.c)
- Adds request processing layers such as logging or security checks.

### Thread pool
- [src/threadpool/threadpool.c](../src/threadpool/threadpool.c)
- Handles incoming client connections in worker threads.

### Socket layer
- [src/net/socket.c](../src/net/socket.c)
- Creates and manages the listening socket.

### Public interface
- [include/server.h](../include/server.h)
- Defines shared structs and function declarations used across the project.

## 5. How a request flows through the server

1. The main server starts and creates a listening socket.
2. It registers default routes and middleware.
3. The event loop accepts new connections.
4. Each connection is handed to the thread pool.
5. The request is parsed into a struct.
6. The router finds the matching handler.
7. Middleware runs before the final handler logic.
8. A response is built and sent back to the client.

This is the core lifecycle you should understand first.

## 6. How to add a new endpoint

To add a new route:
1. Implement a handler function in [src/core/server.c](../src/core/server.c).
2. Register it with the server in the default route setup.
3. Rebuild and test the endpoint with curl.

Example pattern:
```c
void route_example(struct Server *server, struct HttpRequest *request, struct HttpResponse *response, void *context)
{
    server_set_status(response, 200, "OK");
    server_add_header(response, "Content-Type", "application/json");
    snprintf(response->body, sizeof(response->body), "{\"message\":\"hello\"}");
    response->body_length = strlen(response->body);
}
```

Then register it in the server setup:
```c
server_register_route(server, "GET", "/example", route_example, NULL);
```

## 7. How to add middleware

Middleware functions are a good place for logging, auth, rate limiting, or tracing.

The project already wires middleware in the server constructor, so you can add more by following the existing pattern in [src/middleware/middleware.c](../src/middleware/middleware.c).

## 8. Common commands for day-to-day work

```bash
make            # build the server
make parser-test
./server
./examples/demo.sh
```

## 9. Recommended learning path

If you are studying this project from scratch, follow this order:
1. Read [README.md](../README.md) for the high-level overview.
2. Read [include/server.h](../include/server.h) to understand the data structures.
3. Read [src/core/server.c](../src/core/server.c) to see the overall lifecycle.
4. Read [src/http/http.c](../src/http/http.c) for parsing logic.
5. Read [src/router/router.c](../src/router/router.c) for dispatch.
6. Read [src/middleware/middleware.c](../src/middleware/middleware.c) for middleware flow.
7. Read [tests/test_http_parser.c](../tests/test_http_parser.c) to understand expected behavior.
8. Extend the project by adding a new route or middleware.

## 10. Troubleshooting

### Port already in use
If the server cannot bind, stop any old instance and try again.

### Build errors
Make sure GCC and Make are installed and that you are running from the project root.

### Browser access issues
In this environment, localhost forwarding can sometimes be inconsistent. Use curl or the local address directly when testing.

## 11. Next improvements to learn

Once you understand the current structure, these are good next steps:
- keep-alive support
- static file serving
- better error handling
- request body parsing improvements
- JSON helpers
- configuration files
- logging to file

## 12. Summary

This repository is a strong beginner-to-intermediate project for learning systems programming, networking, and backend architecture in C. The main goal is not just to make it run, but to understand how each part contributes to a working server.
