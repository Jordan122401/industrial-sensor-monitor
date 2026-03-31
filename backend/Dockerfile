FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    libglib2.0-dev \
    libsqlite3-dev \
    bluez \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY include/ include/
COPY src/ src/
COPY CMakeLists.txt .

RUN mkdir build && cd build && cmake .. && make

EXPOSE 9000

CMD ["./build/sensor_platform"]
