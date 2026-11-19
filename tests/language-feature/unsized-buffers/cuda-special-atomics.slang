//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -capability cuda_sm_9_0 -validate-ir
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute -capability cuda_sm_9_0 -validate-ir

// These byte-address atomics use wrapper-specific CUDA intrinsics for sized buffers.
// Unsized buffers must instead use pointer atomics, preserving the global address space
// both for direct parameters and when converting an existing sized buffer.
RWByteAddressBuffer sized;
__UnsizedRWByteAddressBuffer direct;
RWStructuredBuffer<uint64_t> results;

[noinline]
void operate(__UnsizedRWByteAddressBuffer buffer, uint index)
{
    float previousFloat;
    double previousDouble;
    uint64_t previousUInt64;
    buffer.InterlockedAddF32(0, 1.0f, previousFloat);
    buffer.InterlockedAddF64(8, 2.0, previousDouble);
    buffer.InterlockedCompareExchangeU64(16, uint64_t(3), uint64_t(5), previousUInt64);
    results[index] = previousUInt64 + uint64_t(previousFloat) + uint64_t(previousDouble);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    operate(sized, 0);
    operate(direct, 1);
}

// CUDA-DAG: uchar * direct
// CUDA-DAG: atomicAdd((slang_bit_cast<float *>(
// CUDA-DAG: atomicAdd((slang_bit_cast<double *>(
// CUDA-DAG: atomicCAS((slang_bit_cast<ulonglong *>(
// CUDA-DAG: void operate{{.*}}(uchar *

// PTX-DAG: atom.global.add.f32
// PTX-DAG: atom.global.add.f64
// PTX-DAG: atom.global.cas.b64
