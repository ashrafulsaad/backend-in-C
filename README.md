# Backend in c

A lightweight HTTP server built from scratch in C, without any external frameworks.

## Features
- Raw socket programming using POSIX APIs
- Handles HTTP GET requests
- Custom request parsing
- Minimal and fast

## Project Structure
\`\`\`
.
├── server.c       # Main server logic
├── server.h       # Header file
└── test.c         # Test file
\`\`\`

## Getting Started

### Prerequisites
- GCC compiler
- Linux / WSL

### Build
\`\`\`bash
gcc server.c -o server
\`\`\`

### Run
\`\`\`bash
./server
\`\`\`

## Author
**Ashrafulsaad** — [GitHub](https://github.com/ashrafulsaad)
