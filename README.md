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

### Prerequisites
- GCC compiler
- Linux / WSL

### Build
```bash
make
```

### Run
```bash
./server
```

### Run parser test
```bash
make parser-test
```

## Author
**Ashrafulsaad** — [GitHub](https://github.com/ashrafulsaad)
