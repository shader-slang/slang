// byte-address-buffer-float3-widen-11591.slang

// Feature test for https://github.com/shader-slang/slang/issues/11545 — slice 2 (#11591).
//
// A `float3` has a 12-byte natural span but a 16-byte std430 *base* alignment. The wide-vs-
// scalarized lowering now decides against the base alignment, so a `float3` promised 16-byte
// alignment lowers to a single wide load (`.xyz` of a 16-byte access) instead of three scalar
// element loads. A promise below the base alignment (but at/above the 4-byte scalar floor) is
// still valid, but scalarizes and is reported as a suboptimal-access warning (E41304).

// Value test: on Vulkan the byte-address buffer lowers to a structured buffer and both the wide
// (promise 16) and scalarized (promise 8) accesses must round-trip the same values. CPU exercises
// the direct byte-address path for the same values.
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

// Codegen test: on SPIR-V the promise-16 access is a single wide `float3` load and the promise-8
// access scalarizes with the below-base warning.
//TEST:SIMPLE(filecheck=CHECKIR):-target spirv -emit-spirv-directly -entry computeMain -stage compute

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):name=scratch
RWByteAddressBuffer scratch;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // Promise 16 == `float3`'s base alignment, so this is a single wide load of a `float3`
    // (three floats loaded together), not three separate scalar loads. No warning.
    scratch.StoreAligned<float3>(0, float3(2.0f, 3.0f, 4.0f));
    float3 wide = scratch.LoadAligned<float3>(0, 16);

    // Promise 8 clears the 4-byte scalar floor but is below the 16-byte base alignment, so the
    // access is valid yet scalarized into three 32-bit element loads, with a below-base warning.
    // CHECKIR-DAG: warning[E41304]
    float3 narrow = scratch.LoadAligned<float3>(0, 8);

    // CHECK: 2.0
    outputBuffer[0] = wide.x;
    // CHECK: 3.0
    outputBuffer[1] = wide.y;
    // CHECK: 4.0
    outputBuffer[2] = wide.z;
    // Scalarized read of the same bytes must agree with the wide read.
    // CHECK: 2.0
    outputBuffer[3] = narrow.x;
    // CHECK: 3.0
    outputBuffer[4] = narrow.y;
    // CHECK: 4.0
    outputBuffer[5] = narrow.z;
}

// The promise-16 load is a single 3-component vector load.
// CHECKIR: OpLoad %v3float
