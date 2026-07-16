// byte-address-buffer-consistency-11591.slang

// Feature test for https://github.com/shader-slang/slang/issues/11545 — slice 2 (#11591).
//
// The wide-vs-scalarized lowering decision is driven by the promised `alignment` alone, so a
// constant `location` and a runtime `location` carrying the same `alignment` lower identically.
// Before this change the constant-folded case used the actual offset and could wide-load a
// location that a runtime location with the same promise would have scalarized.
//
// Tested on SPIR-V, where byte-address accesses lower to structured access and the alignment
// promise drives codegen. (On HLSL the access stays a native `ByteAddressBuffer.Load`, so the
// wide-vs-scalarized distinction is not observable.)

//TEST:SIMPLE(filecheck=CHECK):-target spirv -emit-spirv-directly -entry computeMain -stage compute

RWByteAddressBuffer buffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // Promise 8 clears `float4`'s 4-byte scalar floor but is below its 16-byte base alignment, so
    // the access is valid yet scalarized into four 32-bit element loads — even though the constant
    // location 16 is itself 16-aligned. The below-base promise is reported as a suboptimal-access
    // warning (E41304), and the constant and runtime locations must still lower identically.
    // CHECK-DAG: warning[E41304]
    float4 a = buffer.LoadAligned<float4>(16, 8);

    // A runtime location with the same promise must lower identically — four 32-bit loads.
    // CHECK-DAG: warning[E41304]
    float4 b = buffer.LoadAligned<float4>(tid.x * 16, 8);

    buffer.Store<float4>(0, a + b, 16);
}

// Each promise-8 `float4` load scalarizes into four 32-bit element loads (eight total across the
// two loads); the constant-location load no longer emits a single wide vector load.
// CHECK-COUNT-8: OpLoad %float
