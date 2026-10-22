// byte-address-buffer-grouped-fxc-11593.slang

// Feature test for https://github.com/shader-slang/slang/issues/11593. With an HLSL profile of
// Shader Model 5.0 or earlier, Slang emits byte-address accesses of non-`uint` scalars as `uint`
// accesses plus a bit-cast (`useBitCastFromUInt`). A partially covered vector access (promise 8
// covers a `float2` but not the whole 16-byte `float4`) is therefore emitted as four `uint`
// accesses there, never as typed `float2` groups.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute -profile cs_5_0

RWByteAddressBuffer buffer;

[shader("compute")]
[numthreads(1, 1, 1)]
// A typed group would appear as a `Load<float2 >` or as a `float2(...)` store payload.
// CHECK-LABEL: void computeMain
// CHECK-NOT: Load<
// CHECK-NOT: float2
// CHECK-COUNT-4: asfloat(
// CHECK-NOT: float2
// CHECK-COUNT-4: Store({{.*}}asuint(
// CHECK-NOT: float2
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint loc = tid.x * 16;

    float4 a = buffer.LoadAligned<float4>(loc, 8);

    buffer.Store<float4>(0, a, 8);
}
