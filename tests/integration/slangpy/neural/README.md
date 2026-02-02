# Neural Module SlangPy Integration Tests

This directory contains integration tests for the Slang `neural` module using SlangPy as the test harness.

## Overview

These tests verify that the experimental `neural` module works correctly when used through SlangPy. They cover:

- **Bindless resources**: Pointer types and DescriptorHandle resources
- **FFLayer forward/backward passes**: Both inline vector and cooperative matrix backends
- **Training convergence**: End-to-end training tests for simple regression tasks
- **Autodiff**: Backward differentiation through neural network layers

## Requirements

- Python 3.10+
- slangpy (`pip install slangpy`)
- pytest (`pip install pytest pytest-xdist`)
- numpy
- GPU with Vulkan and/or CUDA support

## Running Tests

### Run all neural integration tests:

```bash
cd /path/to/slang
pytest tests/integration/slangpy/neural/ -v
```

### Run specific test file:

```bash
pytest tests/integration/slangpy/neural/test_neural_bindless.py -v
```

### Run with specific device type:

```bash
# Vulkan only
SLANGPY_DEVICE_TYPES=vulkan pytest tests/integration/slangpy/neural/ -v

# CUDA only
SLANGPY_DEVICE_TYPES=cuda pytest tests/integration/slangpy/neural/ -v
```

## Test Files

| File                                       | Description                                               |
| ------------------------------------------ | --------------------------------------------------------- |
| `test_neural_bindless.py`                  | Tests bindless pointer and DescriptorHandle resources     |
| `test_neural_bwd_diff_smoke.py`            | Smoke test for autodiff backward pass                     |
| `test_neural_frontend_training.py`         | FFLayer training with InlineVector backend                |
| `test_neural_coopmat_frontend_training.py` | FFLayer training with CoopMat (WaveTangledVector) backend |

## Slang Modules

| File                                           | Description                        |
| ---------------------------------------------- | ---------------------------------- |
| `test_neural_bindless_pointer.slang`           | Simple pointer read test           |
| `test_neural_bindless_descriptor_handle.slang` | DescriptorHandle compute shader    |
| `test_neural_frontend_training.slang`          | FFLayer with InlineVector          |
| `test_neural_coopmat_frontend_training.slang`  | FFLayer with WaveTangledVector     |
| `fflayer-bug-repro.slang`                      | Autodiff backward pass test module |

## CI Integration

These tests are designed to be run as part of slang's CI pipeline. They require:

1. slangpy to be installed (latest release from PyPI)
2. The freshly built slang libraries to be copied over the slangpy bundled libraries
3. A GPU with appropriate driver support

See `.github/workflows/ci-slang-test.yml` for the CI integration pattern.
