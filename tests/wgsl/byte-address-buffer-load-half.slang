//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CONST): -target wgsl -entry constantOffsetMain -stage compute

// WGSL has `f16` but no 16-bit integer types. A byte-address-buffer load of
// `half` data (scalars, vectors, and struct fields) must be lowered through
// the containing 32-bit word, bit-cast to `vec2<f16>`, instead of through a
// 16-bit integer.

// CHECK-NOT: u16
// CHECK: enable f16;
// CHECK-NOT: u16
// CHECK: var<storage, read> pool_{{.*}} : array<u32>;
// CHECK-NOT: u16
// CHECK: fn computeMain
// CHECK-NOT: u16
// CHECK: bitcast<vec2<f16>>
// CHECK-NOT: u16

// With constant offsets the selected half-word is visible: the low half of a word is shifted by
// 0 and the high half by 16, for scalars and for `Record` fields at 2-byte offsets.
// CONST-NOT: u16
// CONST: fn constantOffsetMain
// CONST-NOT: u16
// CONST: pool_{{[0-9]+}}[(u32(0))/4]
// CONST-NEXT: bitcast<vec2<f16>>(({{.*}} >> (u32(0))))
// CONST-NOT: u16
// CONST: pool_{{[0-9]+}}[(u32(0))/4]
// CONST-NEXT: bitcast<vec2<f16>>(({{.*}} >> (u32(16))))
// CONST-NOT: u16
// CONST: pool_{{[0-9]+}}[(u32(256))/4]
// CONST-NEXT: bitcast<vec2<f16>>(({{.*}} >> (u32(0))))
// CONST-NOT: u16
// CONST: pool_{{[0-9]+}}[(u32(256))/4]
// CONST-NEXT: bitcast<vec2<f16>>(({{.*}} >> (u32(16))))
// CONST-NOT: u16
// CONST: pool_{{[0-9]+}}[(u32(260))/4]
// CONST-NEXT: bitcast<f32>(
// CONST-NOT: u16
// CONST: pool_{{[0-9]+}}[(u32(264))/4]
// CONST-NEXT: bitcast<vec2<f16>>(({{.*}} >> (u32(0))))
// CONST-NOT: u16
// CONST: pool_{{[0-9]+}}[(u32(264))/4]
// CONST: bitcast<vec2<f16>>(({{.*}} >> (u32(16))))
// CONST-NOT: u16

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

[shader("compute")]
[numthreads(1, 1, 1)]
void constantOffsetMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    const half low = pool.Load<half>(0);
    const half high = pool.Load<half>(2);
    const Record r = pool.Load<Record>(256);
    outputBuffer[dispatchThreadID.x] = float4(
        float(low),
        float(high),
        float(r.a) + float(r.b),
        float(r.d.x) + float(r.d.y) + r.c);
}
