#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

make >/dev/null
./server > /tmp/c-http-demo.log 2>&1 &
SERVER_PID=$!
trap 'kill $SERVER_PID >/dev/null 2>&1 || true' EXIT

sleep 1

echo "=== Root page ==="
curl -s http://127.0.0.1:18080/ | head -n 10

echo

echo "=== Health endpoint ==="
curl -s http://127.0.0.1:18080/health

echo

echo "=== Echo endpoint ==="
curl -s -X POST http://127.0.0.1:18080/api/echo -d 'hello=world'
echo
