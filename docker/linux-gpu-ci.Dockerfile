# Linux GPU CI Container Image
#
# Base image with CUDA 12.4.1 for GPU testing on self-hosted runners
# with driver 550.54.15 (doesn't support newer PTX versions from CUDA 12.9+)
#
# Used by:
# - .github/workflows/ci-slang-build-container.yml
# - .github/workflows/ci-slang-test-container.yml
# - .github/workflows/copilot-setup-steps.yml
#
# Build and push:
#   docker build -f docker/linux-gpu-ci.Dockerfile -t ghcr.io/shader-slang/slang-linux-gpu-ci:12.4.1 .
#   docker push ghcr.io/shader-slang/slang-linux-gpu-ci:12.4.1

FROM nvidia/cuda:12.4.1-devel-ubuntu22.04

# Install essential tools required for GitHub Actions and Copilot
# - curl: for downloading Copilot runtime and dependencies
# - wget: for downloading CMake and other tools
# - git: for repository operations and submodules
# - tar, gzip: for archive extraction
# - ca-certificates: for HTTPS downloads
RUN apt-get update && apt-get install -y \
    curl \
    wget \
    git \
    tar \
    gzip \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    ninja-build \
    unzip \
    python3 \
    python3-pip \
    python-is-python3 \
    libx11-dev \
    libxcursor-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxi-dev \
    libgl1-mesa-dev \
    libvulkan-dev \
    vulkan-validationlayers \
    spirv-tools \
    glslang-tools \
    && rm -rf /var/lib/apt/lists/*

# Install runtime libraries for test execution
RUN apt-get update && apt-get install -y \
    libx11-6 \
    libxext6 \
    libegl1 \
    libvulkan1 \
    && rm -rf /var/lib/apt/lists/*

# Install CMake 3.30 (required for CMakePresets.json version 6)
RUN wget -q https://github.com/Kitware/CMake/releases/download/v3.30.0/cmake-3.30.0-linux-x86_64.tar.gz && \
    tar -xzf cmake-3.30.0-linux-x86_64.tar.gz -C /opt && \
    rm cmake-3.30.0-linux-x86_64.tar.gz && \
    ln -s /opt/cmake-3.30.0-linux-x86_64/bin/cmake /usr/local/bin/cmake && \
    ln -s /opt/cmake-3.30.0-linux-x86_64/bin/ctest /usr/local/bin/ctest && \
    ln -s /opt/cmake-3.30.0-linux-x86_64/bin/cpack /usr/local/bin/cpack

# Git configuration for container workflows
RUN git config --global --add safe.directory '*'

# Verify installations
RUN echo "=== Installed Tools ===" && \
    echo "curl: $(curl --version | head -1)" && \
    echo "git: $(git --version)" && \
    echo "cmake: $(cmake --version | head -1)" && \
    echo "nvcc: $(nvcc --version | grep -i 'release')"

# Set labels for identification
LABEL org.opencontainers.image.source=https://github.com/shader-slang/slang
LABEL org.opencontainers.image.description="Slang Linux GPU CI container with CUDA 12.4.1"
LABEL org.opencontainers.image.licenses=MIT
