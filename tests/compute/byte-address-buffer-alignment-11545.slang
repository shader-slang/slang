// byte-address-buffer-alignment-11545.slang

// Diagnostic test for https://github.com/shader-slang/slang/issues/11545
//
// The `alignment` argument of `(RW)ByteAddressBuffer.LoadAligned`/`Store` is a compile-time
// contract: it must be a compile-time constant, a power of two, and at least the alignment of
// `T`'s scalar components, and a compile-time-constant `location` must be a multiple of it.
// Each violation is reported as a distinct diagnostic.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute

RWByteAddressBuffer buffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint dynamicAlignment = tid.x;

    // A non-constant alignment cannot be honored as a compile-time contract.
    // CHECK-DAG: error[E41302]
    float4 a = buffer.LoadAligned<float4>(0, dynamicAlignment);

    // A non-power-of-two alignment is rejected.
    // CHECK-DAG: error[E41301]
    float4 b = buffer.LoadAligned<float4>(0, 12);

    // A compile-time-constant location that is not a multiple of the alignment is a
    // self-contradictory call.
    // CHECK-DAG: error[E41303]
    float4 c = buffer.LoadAligned<float4>(20, 16);

    // An alignment smaller than the scalar component (a `float` requires 4 bytes, 2 was given).
    // CHECK-DAG: error[E41300]
    float4 d = buffer.LoadAligned<float4>(0, 2);

    // The scalar-component check also applies to already-legal scalar accesses that do not need
    // legalization (a `float` load/store): an alignment of 2 is still below the 4-byte requirement.
    // CHECK-DAG: error[E41300]
    float e = buffer.LoadAligned<float>(0, 2);
    // CHECK-DAG: error[E41300]
    buffer.Store<float>(0, e, 2);

    buffer.Store<float4>(0, a + b + c + d);
}
