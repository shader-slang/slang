// byte-address-buffer-chunked-fxc-11803.slang

// Regression test for shader-slang/slang#11803. fxc-era HLSL (DX profile <= 5.0) has no templated
// `.Load<T>`/`.Store<T>` on byte-address buffers, so `-profile cs_5_0` sets `useBitCastFromUInt`.
// A partially-covered vector access (promise 8 covers a `float2` but not the whole 16-byte
// `float4`) must scalarize into `uint`+`asfloat`/`asuint` there, never emit typed `float2` chunks.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute -profile cs_5_0

RWByteAddressBuffer buffer;

[shader("compute")]
[numthreads(1, 1, 1)]
// These `CHECK-NOT`s span from the entry point to the first scalarized `Load(`. A regressed chunker
// emits the typed chunk load right there — before any scalar `Load(` — so a `Load<`/`float2`/`float4`
// in that window trips them. The store side is covered positively by the scalar `Store(`+`asuint`
// below (a typed vector store would be `.Store(..., float2(...))`, so no scalar `asuint` would appear).
// CHECK: void computeMain
// CHECK-NOT: Load<
// CHECK-NOT: Store<
// CHECK-NOT: float2
// CHECK-NOT: float4
// CHECK: Load(
// CHECK: asfloat
// CHECK: Store(
// CHECK: asuint
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint loc = tid.x * 16;

    // Promise 8 covers a `float2` but not the whole `float4`: chunk-capable targets emit two
    // `float2` accesses, but fxc must scalarize into four `uint`+`asfloat` element loads.
    float4 a = buffer.LoadAligned<float4>(loc, 8);

    // Store mirror: four `(uint)asuint(...)` element stores, never a typed vector `.Store`.
    buffer.Store<float4>(0, a, 8);
}
