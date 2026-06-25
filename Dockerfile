# Stage 1: Build
FROM ubuntu:22.04 AS builder
RUN apt-get update && apt-get install -y \
    cmake \
    g++ \
    make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN mkdir build && cd build && cmake .. && make server

# Stage 2: Run
FROM ubuntu:22.04

WORKDIR /app

COPY --from=builder /app/build/server .

EXPOSE 7379

CMD ["./server"]
