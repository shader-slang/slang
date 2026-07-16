// byte-address-buffer-consistency-11591.slang

// Feature test for https://github.com/shader-slang/slang/issues/11545 — slice 2 (#11591).
//
// The wide-vs-scalarized lowering decision is driven by the promised `alignment` alone, so a
// constant `location` and a runtime `location` carrying the same `alignment` lower identically.
// Before this change the constant-folded case used the actual offset and could wide-load a
// location that a runtime location with the same promise would have scalarized.

// Value test: read the same 16-aligned bytes through both a constant location and a runtime
// location, each promising 8 (below `float4`'s 16-byte base alignment, so both scalarize). The two
// reads must agree and round-trip the stored value. Vulkan exercises the structured-buffer path;
// CPU exercises the direct path.
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

// Codegen test: on SPIR-V the promise-8 `float4` accesses scalarize (four 32-bit loads each) and
// the below-base promise warns; the constant-location load no longer emits a single wide load.
//TEST:SIMPLE(filecheck=CHECKIR):-target spirv -emit-spirv-directly -entry computeMain -stage compute

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):name=scratch
RWByteAddressBuffer scratch;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    scratch.StoreAligned<float4>(16, float4(5.0f, 6.0f, 7.0f, 8.0f));

    // Promise 8 clears `float4`'s 4-byte scalar floor but is below its 16-byte base alignment, so
    // the access is valid yet scalarized into four 32-bit element loads — even though the constant
    // location 16 is itself 16-aligned. The below-base promise is reported as a suboptimal-access
    // warning (E41304), and the constant and runtime locations must still lower identically.
    // CHECKIR-DAG: warning[E41304]
    float4 a = scratch.LoadAligned<float4>(16, 8);

    // A runtime location with the same promise must lower identically — four 32-bit loads — and
    // read back the same value.
    // CHECKIR-DAG: warning[E41304]
    float4 b = scratch.LoadAligned<float4>((tid.x + 1) * 16, 8);

    // The two reads of the same bytes must agree; emit their component-wise sum halved (== the
    // stored value) so a divergence between the constant and runtime paths would show up.
    float4 avg = (a + b) * 0.5f;
    // CHECK: 5.0
    outputBuffer[0] = avg.x;
    // CHECK: 6.0
    outputBuffer[1] = avg.y;
    // CHECK: 7.0
    outputBuffer[2] = avg.z;
    // CHECK: 8.0
    outputBuffer[3] = avg.w;
}

// Each promise-8 `float4` load scalarizes into four 32-bit element loads (eight total across the
// two loads); the constant-location load no longer emits a single wide vector load.
// CHECKIR-COUNT-8: OpLoad %float
