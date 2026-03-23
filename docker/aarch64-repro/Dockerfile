FROM arm64v8/ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    libx11-dev \
    curl \
    python3 \
    && rm -rf /var/lib/apt/lists/*

RUN gcc --version && cmake --version && ninja --version

WORKDIR /workspace
