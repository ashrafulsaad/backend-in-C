FROM gcc:13-bookworm AS builder
WORKDIR /app
COPY . .
RUN make

FROM debian:bookworm-slim AS runtime
WORKDIR /app
COPY --from=builder /app/server .
EXPOSE 80
CMD ["./server"]
