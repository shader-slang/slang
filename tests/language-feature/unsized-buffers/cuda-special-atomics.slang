//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -capability cuda_sm_9_0 -validate-ir
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute -capability cuda_sm_9_0 -validate-ir

// These byte-address atomics use CUDA prelude helpers. Sized and unsized buffer structs
// must both emit the same atomic operations, including when an unsized buffer is passed
// through a helper after converting an existing sized buffer.
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

// CUDA-DAG: UnsizedRWByteAddressBuffer direct
// CUDA-DAG: atomicAdd({{.*}}_getPtrAt<float>
// CUDA-DAG: atomicAdd({{.*}}_getPtrAt<double>
// CUDA-DAG: atomicCAS({{.*}}_getPtrAt<uint64_t>
// CUDA-DAG: void operate{{.*}}(UnsizedRWByteAddressBuffer

// PTX-DAG: atom{{(\.global)?}}.add.f32
// PTX-DAG: atom{{(\.global)?}}.add.f64
// PTX-DAG: atom{{(\.global)?}}.cas.b64
