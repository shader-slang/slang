# Local Linux Docker Development for Slang

This guide explains how to use Docker containers to build and test Slang locally using the same environment as the CI runners.

## Table of Contents

- [Introduction](#introduction)
- [Prerequisites](#prerequisites)
- [Quick Start](#quick-start)
- [Build Options](#build-options)
- [LLVM Backend Options](#llvm-backend-options)
- [Test Options](#test-options)
- [Troubleshooting](#troubleshooting)
- [Advanced Usage](#advanced-usage)
- [Comparison with CI](#comparison-with-ci)

## Introduction

The Slang project uses self-hosted Linux GPU runners with a custom Docker container for CI testing. These scripts allow you to:

- Build Slang in the exact same environment as CI
- Run tests locally without setting up dependencies
- Debug CI failures on your local machine
- Test GPU features without a physical GPU (for CPU-only tests)

**Container Image**: `ghcr.io/shader-slang/slang-linux-gpu-ci:v1.3.0`

**What's included**:

- CUDA 13.0.1 development environment
- Vulkan SDK 1.4.321.1
- CMake 3.30.0, Ninja, GCC
- Graphics libraries (X11, Mesa, Vulkan)
- SPIRV validation tools

## Prerequisites

### Required

- **Docker 20.10+** - For container support

  ```bash
  # Install Docker (Ubuntu/Debian)
  curl -fsSL https://get.docker.com -o get-docker.sh
  sudo sh get-docker.sh

  # Add yourself to docker group (to avoid sudo)
  sudo usermod -aG docker $USER
  # Log out and back in for this to take effect
  ```

### Optional (for GPU tests)

- **NVIDIA Container Runtime** - For GPU passthrough

  ```bash
  # Install nvidia-container-toolkit (Ubuntu/Debian)
  distribution=$(. /etc/os-release;echo $ID$VERSION_ID)
  curl -s -L https://nvidia.github.io/nvidia-docker/gpgkey | sudo apt-key add -
  curl -s -L https://nvidia.github.io/nvidia-docker/$distribution/nvidia-docker.list | \
    sudo tee /etc/apt/sources.list.d/nvidia-docker.list
  sudo apt-get update
  sudo apt-get install -y nvidia-container-toolkit
  sudo systemctl restart docker
  ```

- **NVIDIA Driver 550+** - For CUDA 13.0 support
  - Check version: `nvidia-smi`

### Storage

- Minimum 16GB free disk space
- Docker images are ~5GB
- Build artifacts are ~2-4GB per configuration

## Quick Start

### Build Slang

```bash
# Build in Debug configuration (fastest, no LLVM)
./extras/docker-build.sh

# Build in Release configuration
./extras/docker-build.sh --config release

# Build with LLVM support (downloads prebuilt)
./extras/docker-build.sh --llvm fetch
```

### Run Tests

```bash
# Run quick CPU tests (no GPU required)
./extras/docker-test.sh --category quick

# Run specific test file
./extras/docker-test.sh tests/language-feature/lambda/lambda-0.slang

# Run full CI test suite (requires GPU)
./extras/docker-test.sh --category full
```

## Build Options

### Configuration Selection

**`--config <debug|release>`** (default: `debug`)

- `debug` - Debug build with symbols, slower execution
- `release` - Optimized build, faster execution, no debug symbols

Example:

```bash
./extras/docker-build.sh --config release
```

### LLVM Backend Mode

**`--llvm <disable|fetch|cached>`** (default: `disable`)

See [LLVM Backend Options](#llvm-backend-options) for details.

### Clean Build

**`--clean`** - Remove build directory before building

```bash
./extras/docker-build.sh --clean
```

Use this when:

- Switching between LLVM modes
- CMake configuration changes
- Build artifacts are corrupted

### Skip Container Pull

**`--no-pull`** - Use local container image without pulling

```bash
./extras/docker-build.sh --no-pull
```

Use this when:

- Working offline
- Using a custom container image
- Image is already up to date

### GitHub Token

**`--github-token <token>`** - Provide GitHub token for API access

```bash
./extras/docker-build.sh --llvm fetch --github-token ghp_xxxxxxxxxxxx

# Or via environment variable
export GITHUB_TOKEN=ghp_xxxxxxxxxxxx
./extras/docker-build.sh --llvm fetch
```

Needed for:

- Higher API rate limits when fetching LLVM binaries
- Accessing private repositories (if any)

## LLVM Backend Options

Slang supports multiple LLVM backend configurations. Choose based on your needs:

### `disable` (Default) ⚡ Fastest

**Best for**: Local development, GPU testing, quick iteration

```bash
./extras/docker-build.sh --llvm disable
```

**Pros**:

- Fastest builds
- Matches GPU CI configuration
- No network access needed
- Minimal dependencies

**Cons**:

- No LLVM target support
- Cannot generate LLVM IR
- Cannot use LLVM-based optimizations

**When to use**:

- General development work
- Testing non-LLVM targets (SPIRV, HLSL, etc.)
- Debugging compilation issues
- Quick iteration cycles

### `fetch` 📦 Recommended for Full Features

**Best for**: Full-featured local builds with LLVM support

```bash
./extras/docker-build.sh --llvm fetch
```

**How it works**:

1. First build: Downloads prebuilt slang-llvm (~100MB) from GitHub releases
2. CMake caches the binary locally
3. Subsequent builds: Reuses cached binary (fast)

**Pros**:

- Full LLVM backend support
- Fast after first download
- Automatic version matching
- No manual LLVM build needed

**Cons**:

- Requires internet connection on first use
- GitHub API rate limits (use `--github-token` to avoid)
- Binary may not be available for all platforms

**When to use**:

- Need LLVM target support
- Testing LLVM IR generation
- Working on LLVM-based optimizations
- Want full feature parity with release builds

### `cached` 🔧 Advanced

**Best for**: LLVM backend development

```bash
# First, build LLVM (one-time)
./external/build-llvm.sh --install-prefix $(pwd)/build/llvm-project-install

# Then use it in container builds
./extras/docker-build.sh --llvm cached
```

**How it works**:

- Mounts pre-built LLVM from `build/llvm-project-install/` into container
- Container uses host's LLVM installation
- Matches non-container CI behavior

**Pros**:

- Full control over LLVM version
- Can modify LLVM and rebuild
- Matches advanced CI workflow
- Offline after initial build

**Cons**:

- Requires building LLVM first (significant time investment)
- Extra disk space for LLVM build (several GB)
- Manual LLVM version management

**When to use**:

- Developing LLVM backend features
- Testing custom LLVM modifications
- Debugging LLVM integration issues
- Need specific LLVM version/configuration

## Test Options

### Test Categories

**`--category <quick|full|smoke>`** (default: `quick`)

#### `quick` - Fast Tests ⚡

**GPU Required**: No (automatically skips GPU APIs if no GPU detected)

```bash
./extras/docker-test.sh --category quick
```

**What it runs**:

- Core language feature tests
- Parser and semantic analysis tests
- Basic code generation tests
- Compute tests (CPU or GPU depending on availability)
- Quick Vulkan/CUDA tests (if GPU available)

**Behavior**:

- **With GPU**: Runs quick subset of tests including GPU APIs (Vulkan, CUDA)
- **Without GPU** (`--no-gpu`):
  - Uses `-api cpu -skip-api-detection` to restrict to CPU-only tests
  - Applies `tests/expected-failure-no-gpu.txt` for known GPU-dependent failures

**Note**: The container has Vulkan libraries installed, so we explicitly restrict to CPU API and skip API detection in CPU-only mode to prevent GPU initialization attempts.

**When to use**:

- Quick validation after changes
- Pre-commit testing
- Fast feedback loop
- Development with or without GPU

#### `full` - Complete CI Test Suite 🚀

**GPU Required**: Yes

```bash
./extras/docker-test.sh --category full
```

**What it runs**:

- All tests from `quick` category
- GPU compute tests (CUDA, Vulkan, D3D)
- SPIRV validation
- Render tests
- Integration tests

**Matches CI**:

- Uses same test categories
- Uses same expected failure lists
- Same SPIRV validation settings

**When to use**:

- Before creating PR
- Debugging CI failures
- Testing GPU features
- Final validation

#### `smoke` - Minimal Sanity Checks 💨

**GPU Required**: No

```bash
./extras/docker-test.sh --category smoke
```

**What it runs**:

- Basic compilation tests
- Minimal functionality checks
- Sanity tests only

**When to use**:

- Verifying build succeeded
- Quick sanity check
- After major refactoring

### Running Specific Tests

**Test by path prefix**:

```bash
# Run all lambda tests
./extras/docker-test.sh tests/language-feature/lambda/

# Run single test file
./extras/docker-test.sh tests/language-feature/lambda/lambda-0.slang

# Run all tests in directory
./extras/docker-test.sh tests/language-feature/
```

### Test Server Configuration

**`--server-count <n>`** (default: `4`)

```bash
./extras/docker-test.sh --server-count 8
```

**What it does**:

- Runs tests in parallel using test server pool
- Higher count = more parallelism = faster tests
- Limited by CPU cores

**Guidelines**:

- `1`: Serial execution, useful for debugging
- `4`: Default, good for 4-8 core systems
- `8`: Faster, good for 8+ core systems
- `nproc`: Maximum parallelism (use with caution)

### GPU Adapter Information

**`--show-adapter-info`** - Display GPU adapter details

```bash
./extras/docker-test.sh --show-adapter-info --category full
```

**What it shows**:

- GPU name and memory
- Driver version
- CUDA version
- Vulkan capabilities

**Use for**:

- Debugging GPU issues
- Verifying driver version
- Understanding test environment

### CPU-Only Mode

**`--no-gpu`** - Force CPU-only testing

```bash
./extras/docker-test.sh --no-gpu
```

**Auto-enabled when**:

- No NVIDIA GPU detected
- nvidia-smi not available
- Running `--category quick`

**Manual override**:

- Use when GPU is available but you want CPU tests only

## Troubleshooting

### Docker Not Found

**Error**: `Docker not found. Please install Docker first.`

**Solution**:

```bash
# Install Docker
curl -fsSL https://get.docker.com -o get-docker.sh
sudo sh get-docker.sh

# Verify installation
docker --version
```

### Docker Permission Denied

**Error**: `Docker daemon not running or permission denied`

**Solution**:

```bash
# Add yourself to docker group
sudo usermod -aG docker $USER

# Log out and back in, then verify
docker ps
```

### GPU Not Accessible

**Error**: GPU detected but tests fail

**Checklist**:

1. Install NVIDIA Container Runtime:

   ```bash
   sudo apt-get install -y nvidia-container-toolkit
   sudo systemctl restart docker
   ```

2. Verify GPU access in Docker:

   ```bash
   docker run --rm --gpus all nvidia/cuda:12.5.1-base-ubuntu22.04 nvidia-smi
   ```

3. Check driver version:
   ```bash
   nvidia-smi
   # Should show driver 550+ for CUDA 13.0 support
   ```

### Vulkan ICD Files Missing

**Warning**: `Vulkan ICD file not found`

**Solution**:

```bash
# Install NVIDIA driver (includes Vulkan ICD)
sudo ubuntu-drivers autoinstall

# Or install specific version
sudo apt-get install nvidia-driver-550

# Verify files exist
ls -la /etc/vulkan/icd.d/nvidia_icd.json
ls -la /usr/share/glvnd/egl_vendor.d/10_nvidia.json
```

### Container Pull Failures

**Error**: `Failed to pull container image`

**Solutions**:

1. **Check internet connection**:

   ```bash
   ping ghcr.io
   ```

2. **Authenticate with GitHub** (if image is private):

   ```bash
   echo $GITHUB_TOKEN | docker login ghcr.io -u USERNAME --password-stdin
   ```

3. **Use `--no-pull` with existing image**:
   ```bash
   ./extras/docker-build.sh --no-pull
   ```

### LLVM Binary Download Failures

**Error**: Unable to find prebuilt binary for slang-llvm

**Solutions**:

1. **Use GitHub token** (avoids rate limits):

   ```bash
   export GITHUB_TOKEN=ghp_xxxxxxxxxxxx
   ./extras/docker-build.sh --llvm fetch
   ```

2. **Use cached LLVM instead**:

   ```bash
   ./external/build-llvm.sh --install-prefix $(pwd)/build/llvm-project-install
   ./extras/docker-build.sh --llvm cached
   ```

3. **Fallback to disable** (fastest):
   ```bash
   ./extras/docker-build.sh --llvm disable
   ```

### LLVM Cache Not Found

**Error**: `LLVM cache not found at: build/llvm-project-install`

**Solution**:

```bash
# Build LLVM locally
./external/build-llvm.sh --install-prefix $(pwd)/build/llvm-project-install

# Wait for completion, then try again
./extras/docker-build.sh --llvm cached
```

### Build Artifacts Not Found

**Error**: `Build artifacts not found`

**Solution**:

```bash
# Clean and rebuild
./extras/docker-build.sh --clean

# Or remove build directory manually
rm -rf build/Debug
./extras/docker-build.sh
```

### Permission Issues with Build Artifacts

**Symptom**: Files owned by root, cannot delete

**Cause**: Container ran as root

**Solution**:

```bash
# Fix ownership
sudo chown -R $(id -u):$(id -g) build/

# Scripts use --user flag to prevent this in future
```

## Advanced Usage

### Running Arbitrary Commands

Use the container for any command:

```bash
docker run --rm \
  -v $(pwd):/workspace \
  -w /workspace \
  ghcr.io/shader-slang/slang-linux-gpu-ci:v1.3.0 \
  bash -c "YOUR_COMMAND_HERE"
```

Examples:

```bash
# Run slangc directly
docker run --rm -v $(pwd):/workspace -w /workspace \
  ghcr.io/shader-slang/slang-linux-gpu-ci:v1.3.0 \
  ./build/Debug/bin/slangc -help

# Check CMake version
docker run --rm ghcr.io/shader-slang/slang-linux-gpu-ci:v1.3.0 \
  cmake --version

# Inspect container
docker run --rm -it ghcr.io/shader-slang/slang-linux-gpu-ci:v1.3.0 bash
```

### Interactive Shell in Container

Debug interactively:

```bash
docker run --rm -it \
  --gpus all \
  -v $(pwd):/workspace \
  -w /workspace \
  ghcr.io/shader-slang/slang-linux-gpu-ci:v1.3.0 \
  bash

# Inside container:
$ cmake --preset default
$ cmake --build --preset debug
$ ./build/Debug/bin/slang-test tests/
$ exit
```

### Using Different Container Versions

```bash
# Specific version
CONTAINER_IMAGE="ghcr.io/shader-slang/slang-linux-gpu-ci:v1.2.0" \
  ./extras/docker-build.sh

# Or edit the script temporarily
```

### Mounting Additional Directories

```bash
docker run --rm \
  -v $(pwd):/workspace \
  -v /path/to/extra:/extra:ro \
  -w /workspace \
  ghcr.io/shader-slang/slang-linux-gpu-ci:v1.3.0 \
  bash -c "ls /extra"
```

### Building LLVM Cache Locally

For `--llvm cached` mode:

```bash
# Build LLVM (one-time)
./external/build-llvm.sh \
  --install-prefix $(pwd)/build/llvm-project-install \
  --repo "https://github.com/llvm/llvm-project"

# Check installation
ls build/llvm-project-install/bin/llvm-config

# Now use cached mode
./extras/docker-build.sh --llvm cached
```

### Debugging Build Failures

```bash
# Enable verbose CMake output
docker run --rm -v $(pwd):/workspace -w /workspace \
  ghcr.io/shader-slang/slang-linux-gpu-ci:v1.3.0 \
  bash -c "cmake --preset default -DCMAKE_VERBOSE_MAKEFILE=ON && \
           cmake --build --preset debug --verbose"

# Or build specific target
docker run --rm -v $(pwd):/workspace -w /workspace \
  ghcr.io/shader-slang/slang-linux-gpu-ci:v1.3.0 \
  bash -c "cmake --preset default && \
           cmake --build --preset debug --target slangc"
```

## Comparison with CI

### What's the Same

✅ **Container Image**: Exact same image as CI
✅ **CMake Configuration**: Same presets and options
✅ **Build Tools**: Same CMake, Ninja, GCC versions
✅ **Test Environment**: Same test server configuration
✅ **SPIRV Validation**: Same validation settings
✅ **Expected Failures**: Same failure list files

### What's Different

| Aspect        | CI                            | Local Docker             |
| ------------- | ----------------------------- | ------------------------ |
| **Platform**  | Self-hosted Linux GPU runners | Your local machine       |
| **GPU**       | Tesla T4                      | Your GPU (or none)       |
| **Storage**   | Fresh on each run             | Persistent build cache   |
| **Network**   | GitHub Actions network        | Your network             |
| **Artifacts** | Uploaded to GitHub            | Local `build/` directory |
| **Caching**   | GitHub Actions cache          | Docker layer cache       |

### LLVM Handling Differences

| Mode        | CI Usage                 | Local Docker        |
| ----------- | ------------------------ | ------------------- |
| **disable** | GPU CI builds            | Default for local   |
| **fetch**   | Not used in CI           | Available locally   |
| **cached**  | Non-GPU CI builds        | Advanced local use  |
| **build**   | Via GitHub Actions cache | Via `build-llvm.sh` |

**CI LLVM Strategy**:

- Master branch: Builds LLVM once, caches in GitHub Actions
- PRs: Reuses cached LLVM from master
- GPU CI: Uses `DISABLE` (doesn't need LLVM)

**Local LLVM Strategy**:

- Default: `disable` (fastest, matches GPU CI)
- Full features: `fetch` (downloads prebuilt)
- Development: `cached` (build once, reuse)

### Testing Differences

**CI runs these test categories**:

- Container builds: `full` category with GPU
- Non-container: Various categories by platform

**Local testing**:

- Default: `quick` (CPU-only, fast)
- Optional: `full` (requires GPU, matches CI)

**Tip**: Use `quick` for development, `full` before PR submission.

---

## See Also

- [Main Build Instructions](../docs/building.md)
- [Testing Guide](../docs/testing.md) (if exists)
- [Container Dockerfile](../docker/linux-gpu-ci.Dockerfile)
- [CI Workflows](../.github/workflows/)

## Questions or Issues?

- Check [Troubleshooting](#troubleshooting) first
- Search [GitHub Issues](https://github.com/shader-slang/slang/issues)
- Ask in [GitHub Discussions](https://github.com/shader-slang/slang/discussions)
