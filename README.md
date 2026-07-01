# Backend in C

A lightweight HTTP server built from scratch in C, without any external frameworks.

## Features
- Raw socket programming using POSIX APIs
- Modular request parsing and response building
- Middleware and routing pipeline
- Threaded request handling
- Basic benchmark and health endpoints

## Project Structure
```text
.
├── Dockerfile
├── Makefile
├── README.md
├── docs/
├── examples/
├── include/
├── src/
├── tests/
└── server            # built binary
```

## Getting Started

For a structured study guide, see [docs/user-guide.md](docs/user-guide.md).

### Prerequisites
- GCC compiler
- Linux / WSL
- curl (for demo checks)

### Build
```bash
make
```

### Run the demo server
```bash
./server
```

### Demo commands you can run
```bash
curl http://127.0.0.1:18080/
curl http://127.0.0.1:18080/health
curl http://127.0.0.1:18080/db
curl -X POST http://127.0.0.1:18080/api/echo -d 'hello=world'
```

### Run parser test
```bash
make parser-test
```

## Learning path
- Start with [docs/user-guide.md](docs/user-guide.md) for guided study.
- Read [docs/architecture.md](docs/architecture.md) for module responsibilities.
- Explore [src/core/server.c](src/core/server.c) and [src/http/http.c](src/http/http.c) to understand request flow.

## Author
**Ashrafulsaad** — [GitHub](https://github.com/ashrafulsaad)
