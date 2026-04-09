# Linux GPU Coverage CI Container Image
#
# Combines GPU CI base (CUDA + Vulkan) with Clang 18 + LLVM coverage tools.
# Used for nightly coverage runs with GPU tests enabled.
#
# Used by:
# - .github/workflows/ci-slang-coverage.yml
#
# Build and push:
#   docker build -f docker/linux-gpu-coverage-ci.Dockerfile -t ghcr.io/shader-slang/slang-linux-gpu-coverage-ci:v1.0.0 .
#   docker push ghcr.io/shader-slang/slang-linux-gpu-coverage-ci:v1.0.0
#
# IMPORTANT: After pushing a new version, update all references in:
#   - .github/workflows/ci-slang-coverage.yml
#
# Find all references:
#   grep -rn "ghcr.io/shader-slang/slang-linux-gpu-coverage-ci" .github/workflows/

FROM nvidia/cuda:13.0.1-devel-ubuntu22.04

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
# Needed for coverage instrumentation (-fprofile-instr-generate -fcoverage-mapping)
# and coverage report generation (llvm-profdata, llvm-cov)
RUN wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key \
      | gpg --dearmor -o /usr/share/keyrings/llvm.gpg && \
    echo "deb [signed-by=/usr/share/keyrings/llvm.gpg] https://apt.llvm.org/jammy/ llvm-toolchain-jammy-18 main" \
      > /etc/apt/sources.list.d/llvm-18.list && \
    apt-get update && apt-get install -y \
    clang-18 \
    llvm-18 \
    && rm -rf /var/lib/apt/lists/*

# Set up clang-18 as the default compiler and symlink LLVM coverage tools
RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-18 100 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-18 100 && \
    update-alternatives --install /usr/bin/llvm-profdata llvm-profdata /usr/bin/llvm-profdata-18 100 && \
    update-alternatives --install /usr/bin/llvm-cov llvm-cov /usr/bin/llvm-cov-18 100 && \
    update-alternatives --install /usr/bin/llvm-symbolizer llvm-symbolizer /usr/bin/llvm-symbolizer-18 100

# Install build dependencies
# Note: Do NOT install libgl1-mesa-dev — it pulls in mesa-vulkan-drivers and libLLVM-15,
# which cause concurrent Vulkan initialization crashes in test-servers (see #10618).
RUN apt-get update && apt-get install -y --no-install-recommends \
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
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

# Install Vulkan SDK 1.4.341.1 from tarball (apt packages discontinued after 1.4.313)
ENV VULKAN_SDK=/opt/vulkan-sdk/1.4.341.1/x86_64
ENV PATH="${VULKAN_SDK}/bin:${PATH}"
ENV LD_LIBRARY_PATH="${VULKAN_SDK}/lib:${LD_LIBRARY_PATH}"
ENV VK_LAYER_PATH="${VULKAN_SDK}/share/vulkan/explicit_layer.d"

RUN wget -q https://sdk.lunarg.com/sdk/download/1.4.341.1/linux/vulkansdk-linux-x86_64-1.4.341.1.tar.xz && \
    tar -xf vulkansdk-linux-x86_64-1.4.341.1.tar.xz && \
    mkdir -p /opt/vulkan-sdk && \
    mv 1.4.341.1 /opt/vulkan-sdk/ && \
    rm -rf vulkansdk-linux-x86_64-1.4.341.1.tar.xz && \
    echo "${VULKAN_SDK}/lib" > /etc/ld.so.conf.d/vulkan-sdk.conf && \
    ldconfig

# Install runtime libraries for test execution
RUN apt-get update && apt-get install -y --no-install-recommends \
    libx11-6 \
    libxext6 \
    libegl1 \
    libvulkan1 \
    && rm -rf /var/lib/apt/lists/*

# Install CMake 3.30 (required for CMakePresets.json version 6)
RUN wget -q https://github.com/Kitware/CMake/releases/download/v3.30.0/cmake-3.30.0-linux-x86_64.tar.gz && \
    echo "09846a3858583f38189b59177586adf125a08c15f3cddcaf7d7d7081ac86969f  cmake-3.30.0-linux-x86_64.tar.gz" | sha256sum -c - && \
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
    echo "clang: $(clang-18 --version | head -1)" && \
    echo "llvm-profdata: $(llvm-profdata-18 --version | head -1)" && \
    echo "llvm-cov: $(llvm-cov-18 --version | head -1)" && \
    echo "llvm-symbolizer: $(llvm-symbolizer-18 --version | head -1)" && \
    echo "git: $(git --version)" && \
    echo "cmake: $(cmake --version | head -1)" && \
    echo "ninja: $(ninja --version)" && \
    echo "nvcc: $(nvcc --version | grep -i 'release')"

# Set labels for identification
LABEL org.opencontainers.image.source=https://github.com/shader-slang/slang
LABEL org.opencontainers.image.description="Slang Linux GPU Coverage CI container with CUDA 13.0.1, Vulkan SDK, and Clang 18 coverage tools"
LABEL org.opencontainers.image.licenses=MIT
