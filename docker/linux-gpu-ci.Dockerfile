# Linux GPU CI Container Image
#
# Base image with CUDA 13.0.1 for GPU testing on self-hosted runners
# Requires driver 580.65.06 or newer
#
# Used by:
# - .github/workflows/ci-slang-build-container.yml
# - .github/workflows/ci-slang-test-container.yml
# - .github/workflows/copilot-setup-steps.yml
#
# Build and push:
#   docker build -f docker/linux-gpu-ci.Dockerfile -t ghcr.io/shader-slang/slang-linux-gpu-ci:v1.3.0 .
#   docker push ghcr.io/shader-slang/slang-linux-gpu-ci:v1.3.0

FROM nvidia/cuda:13.0.1-devel-ubuntu22.04

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
    spirv-tools \
    glslang-tools \
    && rm -rf /var/lib/apt/lists/*

# Install Vulkan SDK 1.4.321.1 from tarball (apt packages discontinued after 1.4.313)
# Using tarball to get the fixed validation layers that resolve cooperative vector issues
ENV VULKAN_SDK=/opt/vulkan-sdk/1.4.321.1/x86_64
ENV PATH="${VULKAN_SDK}/bin:${PATH}"
ENV LD_LIBRARY_PATH="${VULKAN_SDK}/lib:${LD_LIBRARY_PATH}"
ENV VK_LAYER_PATH="${VULKAN_SDK}/share/vulkan/explicit_layer.d"

RUN wget -q https://sdk.lunarg.com/sdk/download/1.4.321.1/linux/vulkansdk-linux-x86_64-1.4.321.1.tar.xz && \
    tar -xf vulkansdk-linux-x86_64-1.4.321.1.tar.xz && \
    mkdir -p /opt/vulkan-sdk && \
    mv 1.4.321.1 /opt/vulkan-sdk/ && \
    rm -rf vulkansdk-linux-x86_64-1.4.321.1.tar.xz

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

# Install environment info script
COPY docker/print-env-info.sh /usr/local/bin/print-env-info
RUN chmod +x /usr/local/bin/print-env-info

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
LABEL org.opencontainers.image.description="Slang Linux GPU CI container with CUDA 12.5.1"
LABEL org.opencontainers.image.licenses=MIT
