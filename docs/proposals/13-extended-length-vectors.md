# SP #13: Extended Length Vectors

This proposal introduces support for vectors with 0 or more than 4 components in
Slang, extending the current vector type system while maintaining compatibility
with existing features.

## Status

Status: Design Review

Implementation: N/A

Author: Ellie Hermaszewska

Reviewer: TBD

## Background

Currently, Slang supports vectors between 1 and 4 components (float1, float2,
float3, float4, (etc for other element types)), following HLSL conventions.
This limitation stems from historical GPU hardware constraints and HLSL's
graphics-focused heritage. However, modern compute applications may require
working with longer vectors for tasks like machine learning, scientific
computing, and select graphics tasks.

Related Work

- C++: std::array provides fixed-size array containers but lacks
  vector-specific operations. SIMD types like std::simd (C++23) support
  hardware-specific vector lengths.

- CUDA: While CUDA doesn't provide native long vectors, libraries like Thrust
  implement vector abstractions. Built-in vector types are limited to 1-4
  components (float1, float2, float3, float4).

- OpenCL: Provides vector types up to 16 components (float2, float4, float8,
  float16, etc.) with full arithmetic and logical operations.

- Modern CPU SIMD: Hardware support for longer vectors continues to grow:
  - Intel AVX-512: 512-bit vectors (16 float32s)
  - ARM SVE/SVE2: Scalable vector lengths up to 2048 bits
  - RISC-V Vector Extensions: Variable-length vector support

These approaches demonstrate different strategies for handling longer vectors,
from fixed-size containers to hardware-specific implementations.

## Motivation

The primary motivation for extended length vectors is to support mathematical
operations and algorithms that naturally operate on higher-dimensional vectors.
While Slang already supports arrays for data storage, certain computations
specifically require vector semantics and operations that arrays don't provide.

A key principle of this proposal is consistency: there is no fundamental
mathematical reason to limit vectors to 4 components. While the current limit
stems from graphics hardware history, modern compute applications shouldn't be
constrained by this arbitrary boundary. Supporting any natural length N
provides a clean, orthogonal design that follows mathematical principles rather
than historical limitations.

Some example use cases:

- Geometric Algebra and Clifford Algebras:

  - 6D vectors for Plücker coordinates (representing lines in 3D space)
  - 6D vectors for screw theory in robotics (combining rotation and translation)
  - 8D vectors for dual quaternions in their vector representation
  - Higher-dimensional geometric products and outer products

- Machine Learning:

  - Neural network feature vectors where vector operations (dot products,
    normalization) are fundamental
  - Distance metrics in high-dimensional embedding spaces
  - Principal Component Analysis with multiple components

- Scientific Computing:
  - Spherical harmonics: Vector operations on coefficient spaces beyond 4D
  - Quantum computing: State vectors in higher-dimensional Hilbert spaces

This extension maintains Slang's mathematical vector semantics while enabling
computations that naturally operate in higher dimensions. The focus is not on
data storage (which arrays already handle) but on preserving vector-specific
operations and mathematical properties in higher dimensions and improving
consistency in the language.

## Proposed Approach

We propose extending Slang's vector type system to support vectors of arbitrary
length, supporting as many of the operations available to 4-vectors as
possible. Hardware limitations will prohibit a complete implementation, for
example certain atomic operations may not be possible on longer vectors.

Key aspects:

- Support vectors of any length that is a natural number, subject to the same
  limits as fixed-length arrays
- Maintain existing syntax for vectors up to length 4
- Maintain numeric swizzling operators.
- Support standard vector operations (add, multiply, etc.)

## Detailed Explanation

### Type System Integration

There are no type system changes required, as the vector type constructor is
already parameterized over arbitrary vector length.

### Operations

#### Component Access:

```slang
vector<float, 8> v;
float f0 = v.x; // First component
float f1 = v[1]; // Array-style access
float2 f3 = v_6_7; // last two member
vector<float, 8> = float2(1,2).xxxxxxxx; // extended swizzling example
```

#### Arithmetic Operations:

Any operations currently supported and generic over restricted vector length
will be made unrestricted.

Part of the scope of this work is to generate a precise list.

For example all componentwise operations will be supported, as well as
reductions.

Cross product will remain restricted to 3-vectors, and will not be overloaded
to 0-vectors or 7-vectors; this is due to worse type inference and error
messages should overloads be added.

#### Atomic Operations:

Not supported

### Storage Operations:

Most platforms restrict the type of data able to be stored in textures, this
proposal does not intend to work-around these restrictions.

### Implementation Details

Memory Layout:

- Vectors are stored in contiguous memory
- Alignment follows platform requirements
- Padding may be required for certain lengths, for example padding to the
  nearest multiple of 4

Performance Considerations:

- No performance degredation for 1,2,3,4-vectors
- SIMD implementation work possible, not initially required

## Alternatives Considered

Fixed Maximum Length:

- Could limit to common sizes (8, 16, 32)
- Simpler implementation but less flexible
- Rejected due to limiting future use cases

Do nothing:

- See motivation section

## Additional notes

### Zero-Length Vectors

This proposal includes support for zero-length vectors. While seemingly
unusual, zero-length vectors are mathematically well-defined and provide
important completeness properties:

- They are the natural result of certain slicing operations
- They serve as the identity element for vector concatenation

### Extended Matrices

While this proposal focuses on vectors, it naturally suggests a future
extension to matrices beyond the current 4x4 limit. Extended matrices would
follow similar principles.

However, extended matrices introduce additional considerations:

- Memory layout and padding strategies for large matrices
- Optimization of common operations (multiplication, transpose)

These matrix-specific concerns are best addressed in a separate proposal that
can build upon the extended vector foundation established here.
