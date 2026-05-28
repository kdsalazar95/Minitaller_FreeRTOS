FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        ca-certificates \
        cmake \
        gcc-arm-none-eabi \
        git \
        libnewlib-arm-none-eabi \
        libnewlib-dev \
        make \
        qemu-system-arm \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

COPY . /workspace

RUN cmake -B build -DCMAKE_BUILD_TYPE=Release -Wno-dev \
    && cmake --build build --parallel

CMD ["qemu-system-arm", "-M", "mps2-an385", "-cpu", "cortex-m3", "-kernel", "build/freertos_rps.elf", "-monitor", "none", "-serial", "stdio", "-nographic"]
