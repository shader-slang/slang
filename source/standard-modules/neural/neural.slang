/**
Neural network primitives for Slang.
This module provides differentiable primitive data structures and operations for implementing
inline MLP (Multilayer Perceptron) in Slang shaders. It includes vector types, storage abstractions,
activation functions, optimizers, and automatic differentiation support for training small inline
neural networks on the GPU.

@remarks EXPERIMENTAL: This module is under active design and may change significantly
or be removed in future versions. DO NOT USE IN PRODUCTION.

@category neural

Features:
- Differentiable vector types (IVector, InlineVector)
- Storage abstractions for GPU buffers (IStorage, StructuredBufferStorage)
- Automatic differentiation support
- Linear transformations with optional bias
- Atomic operations for gradient accumulation

Example Usage:
```
// Create an input vector
InlineVector<float, 4> input = InlineVector<float, 4>({1.0, 2.0, 3.0, 4.0});

// Set up storage for weights
StructuredBufferStorage<float> storage = StructuredBufferStorage<float>(weightsBuffer);

// Perform linear transformation: output = W * input
InlineVector<float, 2> output = input.eval<2, false>(storage, 0);
```
*/
[ExperimentalModule]
module neural;

__include "ivector";
__include "inline-vector";
__include "istorages";
__include "bindless-storage";
__include "buffer-storage";
