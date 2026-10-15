//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute

// WGSL has `f16` but no 16-bit integer types. A byte-address-buffer load of
// `half` data (scalars, vectors, and struct fields) must be lowered through
// the containing 32-bit word, bit-cast to `vec2<f16>`, instead of through a
// 16-bit integer.

// CHECK: enable f16;
// CHECK: var<storage, read> pool_{{.*}} : array<u32>;
// CHECK: fn computeMain
// CHECK: bitcast<vec2<f16>>
// CHECK-NOT: u16

struct Record
{
    half a;
    half b;
    float c;
    half2 d;
};

ByteAddressBuffer pool;
RWStructuredBuffer<float4> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint i = dispatchThreadID.x;
    half h = pool.Load<half>(i * 2);
    half2 h2 = pool.Load<half2>(i * 4 + 64);
    half4 h4 = pool.Load<half4>(128);
    Record r = pool.Load<Record>(i * 12 + 256);

    outputBuffer[i] = float4(
        float(h) + float(h2.y),
        float(h4.x) + float(h4.w),
        float(r.a) + float(r.b) + r.c,
        float(r.d.x) + float(r.d.y));
}
