// byte-address-buffer-pow2-11592.slang

// Diagnostic test for https://github.com/shader-slang/slang/issues/11545 — Slice 3 (#11592).
//
// A non-zero `alignment` must be a power of two and at least the natural alignment of `T`'s
// scalar components. Each violation is reported as a distinct diagnostic.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute

RWByteAddressBuffer buffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // A non-power-of-two alignment is rejected.
    // CHECK-DAG: error[E41301]
    float4 a = buffer.LoadAligned<float4>(0, 12);

    // An alignment smaller than the scalar component (a `float` requires 4 bytes, 2 was given).
    // CHECK-DAG: error[E41300]
    float4 b = buffer.LoadAligned<float4>(0, 2);

    buffer.Store<float4>(0, a + b);
}
