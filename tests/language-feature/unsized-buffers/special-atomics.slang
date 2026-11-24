//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -capability cuda_sm_9_0 -validate-ir
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute -capability cuda_sm_9_0 -validate-ir
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute -DFLOAT32_ONLY=1 -validate-ir
//TEST(compute,metal):COMPARE_COMPUTE(filecheck-buffer=RESULT): -metal -compute -output-using-type -xslang -DFLOAT32_ONLY=1
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=RESULT): -cuda -compute -output-using-type -xslang -DFLOAT32_ONLY=1 -xslang -DSIZED_BINDINGS=1

// Sized and unsized buffers must emit the same byte-address atomic operations, including
// when an unsized buffer is passed through a helper after converting an existing sized buffer.
// Metal supports the F32 operation but not these F64 and U64 CAS APIs. WGSL supports none
// of these floating-point/64-bit atomic operations, so it is excluded from this fixture.
//TEST_INPUT:ubuffer(data=[0x3f800000 0 0 0 0 0], stride=4):name=sized
RWByteAddressBuffer sized;
//TEST_INPUT:ubuffer(data=[0x40000000 0 0 0 0 0], stride=4):name=direct
#ifdef SIZED_BINDINGS
// The CUDA runtime binder uses the sized ABI; operate converts this binding to an unsized view.
RWByteAddressBuffer direct;
#else
__UnsizedRWByteAddressBuffer direct;
#endif
//TEST_INPUT:ubuffer(data=[0], count=4, stride=8):out,name=results
RWStructuredBuffer<uint64_t> results;

[noinline]
void operate(__UnsizedRWByteAddressBuffer buffer, uint index)
{
    float previousFloat;
    buffer.InterlockedAddF32(0, 1.0f, previousFloat);
#ifdef FLOAT32_ONLY
    results[index * 2] = uint64_t(previousFloat);
    results[index * 2 + 1] = uint64_t(buffer.Load<float>(0));
#else
    double previousDouble;
    uint64_t previousUInt64;
    buffer.InterlockedAddF64(8, 2.0, previousDouble);
    buffer.InterlockedCompareExchangeU64(16, uint64_t(3), uint64_t(5), previousUInt64);
    results[index] = previousUInt64 + uint64_t(previousFloat) + uint64_t(previousDouble);
#endif
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

// METAL: void [[OPERATE:operate[a-zA-Z0-9_]*]](uint32_t device*
// METAL: atomic_fetch_add_explicit(((atomic_float device*)
// METAL: [[OPERATE]](sized
// METAL: [[OPERATE]]({{.*}}direct

// RESULT: 1
// RESULT-NEXT: 2
// RESULT-NEXT: 2
// RESULT-NEXT: 3
