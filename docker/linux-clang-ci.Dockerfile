# Linux Clang CI Container Image
#
# Lightweight image with Clang 18 + LLVM tools for sanitizer and coverage CI jobs.
# No CUDA or Vulkan — these jobs run CPU-only tests.
#
# Used by:
# - .github/workflows/ci-slang-sanitizer.yml
# - .github/workflows/ci-slang-coverage.yml  (future)
#
# Build and push:
#   docker build -f docker/linux-clang-ci.Dockerfile -t ghcr.io/shader-slang/slang-linux-clang-ci:v1.0.1 .
#   docker push ghcr.io/shader-slang/slang-linux-clang-ci:v1.0.1

FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install essential tools required for GitHub Actions
RUN apt-get update && apt-get install -y --no-install-recommends \
    curl \
    wget \
    git \
    tar \
    gzip \
    sudo \
    ca-certificates \
    gnupg \
    software-properties-common \
    && rm -rf /var/lib/apt/lists/*

# Install Clang 18 and LLVM tools from the official LLVM APT repository
RUN wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key \
      | gpg --dearmor -o /usr/share/keyrings/llvm.gpg && \
    echo "deb [signed-by=/usr/share/keyrings/llvm.gpg] https://apt.llvm.org/jammy/ llvm-toolchain-jammy-18 main" \
      > /etc/apt/sources.list.d/llvm-18.list && \
    apt-get update && apt-get install -y \
    clang-18 \
    llvm-18 \
    && rm -rf /var/lib/apt/lists/*

# Set up clang-18 as the default compiler
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-18 100 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-18 100 && \
    update-alternatives --install /usr/bin/llvm-profdata llvm-profdata /usr/bin/llvm-profdata-18 100 && \
    update-alternatives --install /usr/bin/llvm-cov llvm-cov /usr/bin/llvm-cov-18 100 && \
    update-alternatives --install /usr/bin/llvm-symbolizer llvm-symbolizer /usr/bin/llvm-symbolizer-18 100

# Install build dependencies
# libx11-dev is needed by common-setup and the Slang build
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    ninja-build \
    python3 \
    python3-pip \
    python-is-python3 \
    libx11-dev \
    nodejs \
    npm \
    && rm -rf /var/lib/apt/lists/*

# Install CMake 3.30 (required for CMakePresets.json version 6)
RUN wget -q https://github.com/Kitware/CMake/releases/download/v3.30.0/cmake-3.30.0-linux-x86_64.tar.gz && \
    echo "09846a3858583f38189b59177586adf125a08c15f3cddcaf7d7d7081ac86969f  cmake-3.30.0-linux-x86_64.tar.gz" | sha256sum -c - && \
    tar -xzf cmake-3.30.0-linux-x86_64.tar.gz -C /opt && \
    rm cmake-3.30.0-linux-x86_64.tar.gz && \
    ln -s /opt/cmake-3.30.0-linux-x86_64/bin/cmake /usr/local/bin/cmake && \
    ln -s /opt/cmake-3.30.0-linux-x86_64/bin/ctest /usr/local/bin/ctest && \
    ln -s /opt/cmake-3.30.0-linux-x86_64/bin/cpack /usr/local/bin/cpack

# Git configuration for container workflows
RUN git config --global --add safe.directory '*'

# Verify installations
RUN echo "=== Installed Tools ===" && \
    echo "clang: $(clang-18 --version | head -1)" && \
    echo "llvm-symbolizer: $(llvm-symbolizer-18 --version | head -1)" && \
    echo "llvm-profdata: $(llvm-profdata-18 --version | head -1)" && \
    echo "llvm-cov: $(llvm-cov-18 --version | head -1)" && \
    echo "git: $(git --version)" && \
    echo "cmake: $(cmake --version | head -1)" && \
    echo "ninja: $(ninja --version)"

# Set labels for identification
LABEL org.opencontainers.image.source=https://github.com/shader-slang/slang
LABEL org.opencontainers.image.description="Slang Linux Clang CI container for sanitizer and coverage jobs"
LABEL org.opencontainers.image.licenses=MIT
