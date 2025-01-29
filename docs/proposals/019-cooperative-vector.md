# SP #019: Cooperative Vector

This proposal introduces support for cooperative vectors in Slang.

## Status

Status: Design Review

Implementation: N/A

Author: Ellie Hermaszewska

Reviewer: TBD

## Background

Slang supports cooperative vector operations, which are optimized for
matrix-vector multiplies, for example in the acceleration of small neural
network evaluations. This design of this feature is based on the SPIR-V
extension SPV_NV_cooperative_vector.

Cooperative vectors are a new set of types that, unlike normal vector types,
have arbitrary length and support a limited set of operations. They are
designed to cooperate behind the scenes when performing matrix-vector
multiplies, without requiring fully occupied subgroups or uniform control flow.

## Semantics

- Cooperative vectors are logically stored in the invocation they belong to,
but can cooperate behind the scenes for matrix-vector operations.

- Unlike cooperative matrices, cooperative vectors don't require a fully
occupied subgroup or uniform control flow, although these conditions can
increase performance.

- The order of arithmetic operations in these functions is
implementation-dependent. The SPIR-V extension specifies that the internal
precision of floating-point operations is defined by the client API.

- Integer operations used in multiplication are performed at the precision of
the result type and are exact (with the usual wrapping rules).

- Cooperative vector types can not (yet) be stored themselves in buffers

## Slang API

### CoopVec Type

The core of this feature is the `CoopVec` type:

```hlsl
struct CoopVec<T : __BuiltinArithmeticType, let N : int> : IArray<T>, IArithmetic
{
    // Zero constructor
    __init();
    // Broadcast 
    __init(T t);
    // Coercion 
    __init<U : __BuiltinArithmeticType>(CoopVec<U, N> other);
    // Variadic component-wise constructor for example CoopVec<int, 3>(1,2,3)
    __init<each U : __BuiltinArithmeticType>(expand each U args) where U == T;

    // Array-like access
    T __subscript(int index);
}
```

For initialization there are several options:

- zero initialization `CoopVec<float, 4>()`
- broadcast initialization `CoopVec<uint8_t, 16>(255)`
- variadic initialization `CoopVec<int, 3>(0, 128, 255)`
- casting initialization `CoopVec<float, 10>(CoopVec<int, 10>())`

It can be indexed with the subscript operator

- `CoopVec<int, 4>(1, 28, 546, 9450)[2] == 546`
- `CoopVec<int, 4>(1, 11, 105, 816)[1] = 12`

Other operations include:

- binary operators `+`, `-`, `*`, `/`, `%`, these behave as elementwise operations
- unary negation `-`
- comparison operators `==`, `!=`, `<`, `>`, `<=`, `>=`, implementing a lexicographic ordering
- `min`, `max`, `clamp`, `step`, `exp`, `log`, `tanh`, `atan`, `fma`, all operating elementwise

It's also possible to set values in mutable cooperative vectors with `fill(T
t)` and `copyFrom(CoopVec<U, N> other)`.

## Basic Usage

### Loading and Storing

Cooperative vectors can by loaded and stored from buffers.

- `[RW]StructuredBuffer`
- `[RW]ByteAddressBuffer`
- `groupshared` arrays

> Note that `StructuredBuffers` are not supported for the HLSL backend

This can be done using the static member function `load(Buffer buffer, int32_t byteOffset)`

For StructuredBuffers and groupshared the type of the buffer element determines
the cooperative vector element type, note that the offset must be a
multiple of the element stride in bytes.

```hlsl
StructuredBuffer<int32_t> inputBuffer;

RWByteAddressBuffer outputBuffer;

func foo()
{
    int myOffsetInBytes = 64;
    // Load a cooperative vector using the type-infering wrapper
    let vecA = coopVecLoad<5>(buffer, myOffsetInBytes);
    
    // Load using the static member function
    let vecB = CoopVec<5, int32_t>.load(buffer); // implicit zero offset

    // Perform operations...
    let vecC = vec + vecB; 

    // Store a cooperative vector
    vecC.store(buffer, 128);
}
```

The full types are as so:

```hlsl
CoopVec<T, N> coopVecLoad<let N : int, T : __BuiltinArithmeticType>(ByteAddressBuffer buffer, int32_t byteOffset = 0);
CoopVec<T, N> coopVecLoad<let N : int, T : __BuiltinArithmeticType>(RWByteAddressBuffer buffer, int32_t byteOffset = 0);
CoopVec<T, N> coopVecLoad<let N : int, T : __BuiltinArithmeticType>(StructuredBuffer<T> buffer, int32_t byteOffset = 0);
CoopVec<T, N> coopVecLoad<let N : int, T : __BuiltinArithmeticType>(RWStructuredBuffer<T> buffer, int32_t byteOffset = 0);
CoopVec<T, N> coopVecLoad<let N : int, T : __BuiltinArithmeticType, let M : int>(__constref groupshared const T[M] data, int32_t byteOffset = 0);
```

> Be aware that the target platform might impose alignment constraints on the
> offset

### Matrix Multiplication

Below is an example of matrix matrix multiplication with bias.

Matrix multiplication operations (`coopVecMatMul`, `coopVecMatMulPacked`,
`coopVecMatMulAdd` and `coopVecMatMulAddPacked`) perform a matrix-vector
multiply where the vector is treated as a column vector and is
left-multiplied by the matrix.

> Please take care to make sure that the buffer interpretations are supported
> by your implementation. Not all platforms support all combinations

The `...Packed` variants are the most general functions, where the user is able
to fully specify the width of the matrix, (although this is strongly dependent
on the `inputInterpretation` parameter). When not using packed inputs, the
matrix width must be equal to the input vector's length, and the
non-`...Packed` variants wrap this common use case.

```slang
StructuredBuffer<int8_t> inputBuffer;
StructuredBuffer<int8_t> matrixBuffer;
StructuredBuffer<int32_t> biasBuffer;
RWStructuredBuffer<int32_t> outputBuffer;

func foo()
{
    let vec = coopVecLoad<4>(inputBuffer);
    // The result type is determined by the first two generic parameters, in
    // this case int32_t and 4,
    let result = coopVecMatMulAdd<int32_t, 4>(
        vec,
        // Matrix buffer interpretation and offset in bytes
        CoopVecComponentType::SignedInt8,
        matrixBuffer,
        0,
        // Bias buffer interpretation and offset in bytes
        CoopVecComponentType::SignedInt8,
        biasBuffer,
        0,
        // Output interpretation
        CoopVecComponentType::SignedInt32,
        // Matrix transposition
        CoopVecMatrixLayout::RowMajor,
        false,
        // matrix stride
        4
    );
    coopVecStore(result, outputBuffer);
}
```

```slang
StructuredBuffer<int32_t> packedInputBuffer;
StructuredBuffer<int8_t> matrixBuffer;
StructuredBuffer<int32_t> biasBuffer;
RWStructuredBuffer<int32_t> outputBuffer;

func bar()
{
    let packedVec = coopVecLoad<1>(packedInputBuffer);
    let k = 4; // 1 * the packing factor
    // The result type is still determined by the first two generic parameters,
    // in this case int32_t and 4,
    let result = coopVecMatMulAddPacked<int32_t, 4>(
        vec,
        // Matrix buffer interpretation and offset in bytes
        CoopVecComponentType::SignedInt8Packed,
        k,
        matrixBuffer,
        0,
        // Bias buffer interpretation and offset in bytes
        CoopVecComponentType::SignedInt8,
        biasBuffer,
        0,
        // Output interpretation
        CoopVecComponentType::SignedInt32,
        // Matrix transposition
        CoopVecMatrixLayout::RowMajor,
        false,
        // matrix stride
        4
    );
    coopVecStore(result, outputBuffer);
}
```

The full types:

```hlsl
[require(cooperative_vector)]
CoopVec<T, M> coopVecMatMulPacked<
    T : __BuiltinArithmeticType,
    let M : int,
    let PackedK : int,
    U : __BuiltinArithmeticType
    >(
    CoopVec<U, PackedK> input,
    constexpr CoopVecComponentType inputInterpretation,
    constexpr int k,
    $(buffer.type) matrix,
    int32_t matrixOffset,
    constexpr CoopVecComponentType matrixInterpretation,
    constexpr CoopVecMatrixLayout memoryLayout,
    constexpr bool transpose,
    constexpr uint matrixStride = 0
);

[require(cooperative_vector)]
CoopVec<T, M> coopVecMatMul<
    T : __BuiltinArithmeticType,
    let M : int,
    let K : int,
    U : __BuiltinArithmeticType
    >(
    CoopVec<U, K> input,
    constexpr CoopVecComponentType inputInterpretation,
    $(buffer.type) matrix,
    int32_t matrixOffset,
    constexpr CoopVecComponentType matrixInterpretation,
    constexpr CoopVecMatrixLayout memoryLayout,
    constexpr bool transpose,
    constexpr uint matrixStride = 0
);

[require(cooperative_vector)]
CoopVec<T, M> coopVecMatMulAddPacked
    <T : __BuiltinArithmeticType,
    let M : int,
    let PackedK : int,
    U : __BuiltinArithmeticType
    >(
    CoopVec<U, PackedK> input,
    constexpr CoopVecComponentType inputInterpretation,
    constexpr int k,
    $(buffer.type) matrix,
    int32_t matrixOffset,
    constexpr CoopVecComponentType matrixInterpretation,
    $(buffer.type) bias,
    int32_t biasOffset,
    constexpr CoopVecComponentType biasInterpretation,
    constexpr CoopVecMatrixLayout memoryLayout,
    constexpr bool transpose,
    constexpr uint matrixStride = 0
);

[require(cooperative_vector)]
CoopVec<T, M> coopVecMatMulAdd<
    T : __BuiltinArithmeticType,
    let M : int,
    let K : int,
    U : __BuiltinArithmeticType
    >(
    CoopVec<U, K> input,
    constexpr CoopVecComponentType inputInterpretation,
    $(buffer.type) matrix,
    int32_t matrixOffset,
    constexpr CoopVecComponentType matrixInterpretation,
    $(buffer.type) bias,
    int32_t biasOffset,
    constexpr CoopVecComponentType biasInterpretation,
    constexpr CoopVecMatrixLayout memoryLayout,
    constexpr bool transpose,
    constexpr uint matrixStride = 0
);
```

There also exist in-place matrix multiplication accumulation member functions,
following the signatures above.

- `matMulAccum`
- `matMulAccumPacked`
- `matMulAddAccum`
- `matMulAddAccumPacked`

### Accumulation Operations

- The `coopVecOuterProductAccumulate` operation computes the outer product of two
vectors and atomically accumulates the result into a buffer

- The `coopVecReduceSumAccumulate` operation performs a component-wise atomic
addition of a vector into a buffer.

```hlsl
void coopVecOuterProductAccumulate<
    T : __BuiltinArithmeticType, 
    let M : int, 
    let N : int
    >(
    CoopVec<T, M> a,
    CoopVec<T, N> b,
    $(buffer.type) matrix,
    int32_t matrixOffset,
    constexpr uint matrixStride,
    constexpr CoopVecMatrixLayout memoryLayout,
    constexpr CoopVecComponentType matrixInterpretation,
)

void coopVecReduceSumAccumulate<
    T : __BuiltinArithmeticType, 
    let N : int
    >(
    CoopVec<T, N> v,
    $(buffer.type) buffer,
    int32_t offset
)
```

> Note that these operations are not accelerated on HLSL

### Enums and Constants

```hlsl
enum CoopVecMatrixLayout
{
    RowMajor,
    ColumnMajor,
    InferencingOptimal,
    TrainingOptimal
};

enum CoopVecComponentType
{
    FloatE4M3,
    FloatE5M2,
    Float16,
    Float32,
    Float64,
    SignedInt8,
    SignedInt16,
    SignedInt32,
    SignedInt64,
    SignedInt8Packed,
    UnsignedInt8,
    UnsignedInt16,
    UnsignedInt32,
    UnsignedInt64,
    UnsignedInt8Packed
};
```

## SPIR-V Translation

Cooperative vector operations in Slang are directly translated to their
corresponding SPIR-V instructions:

- `CoopVec<T, N>` corresponds to `OpTypeCooperativeVectorNV`
- `coopVecLoad` corresponds to `OpCooperativeVectorLoadNV`
- `coopVecStore` corresponds to `OpCooperativeVectorStoreNV`
- `coopVecMatMul` corresponds to `OpCooperativeVectorMatrixMulNV`
- `coopVecMatMulAdd` corresponds to `OpCooperativeVectorMatrixMulAddNV`
- `coopVecOuterProductAccumulate` corresponds to `OpCooperativeVectorOuterProductAccumulateNV`
- `coopVecReduceSumAccumulate` corresponds to `OpCooperativeVectorReduceSumAccumulateNV`

## HLSL Translation

The types and operations are lowered to the experimental API described here:
https://confluence.nvidia.com/pages/viewpage.action?spaceKey=DX&title=CooperativeVectors+aka+Neural+Graphics+for+DX

Please note that SPIR-V is the recommended backend at this time due to
targeting a more stable extension.

## Translation for other targets

The `CoopVec` type is lowered to a fixed size array and cooperative operations
are emulated in each thread.
