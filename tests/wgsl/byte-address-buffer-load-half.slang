//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CONST): -target wgsl -entry constantOffsetMain -stage compute
//TEST:SIMPLE(filecheck=NOERR): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=NOERR): -target wgsl -entry constantOffsetMain -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry constantOffsetMain -stage compute

// WGSL has `f16` but no 16-bit integer types. A byte-address-buffer load of
// `half` data (scalars, vectors, and struct fields) must be lowered through
// the containing 32-bit word, bit-cast to `vec2<f16>`, instead of through a
// 16-bit integer.
//
// byte-address-buffer-load-half-exec.slang runs these loads on WebGPU and Vulkan and checks the
// loaded bits; this file checks the shape of the WGSL output.

// The WGSL emitter writes nothing for a 16-bit integer type and reports E56103, so a regression
// shows up as an error, not as `u16` in the output. NOERR searches the whole output for it, and
// the TINT runs check that the bundled Tint accepts the module.
// NOERR-NOT: {{error\[E[0-9]+\]}}
// NOERR: @compute
// NOERR-NOT: {{error\[E[0-9]+\]}}
// TINT: OpEntryPoint

// CHECK: enable f16;
// CHECK: var<storage, read> pool_{{.*}} : array<u32>;
// CHECK: fn computeMain
// CHECK: bitcast<vec2<f16>>

// With constant offsets the selected half-word is visible: the low half of a word is shifted by
// 0 and the high half by 16, for scalars and for `Record` fields at 2-byte offsets.
// Each loaded word is shifted, bit-cast, and element 0 of the bitcast is the loaded half.
// CONST: fn constantOffsetMain
// CONST: [[W0:_S[0-9]+]] : u32 = pool_{{[0-9]+}}[(u32(0))/4]
// CONST-NEXT: [[H0:_S[0-9]+]] : vec2<f16> = bitcast<vec2<f16>>(([[W0]] >> (u32(0))))
// CONST: [[W1:_S[0-9]+]] : u32 = pool_{{[0-9]+}}[(u32(0))/4]
// CONST-NEXT: [[H1:_S[0-9]+]] : vec2<f16> = bitcast<vec2<f16>>(([[W1]] >> (u32(16))))
// CONST: [[W2:_S[0-9]+]] : u32 = pool_{{[0-9]+}}[(u32(256))/4]
// CONST-NEXT: [[H2:_S[0-9]+]] : vec2<f16> = bitcast<vec2<f16>>(([[W2]] >> (u32(0))))
// CONST: [[W3:_S[0-9]+]] : u32 = pool_{{[0-9]+}}[(u32(256))/4]
// CONST-NEXT: [[H3:_S[0-9]+]] : vec2<f16> = bitcast<vec2<f16>>(([[W3]] >> (u32(16))))
// CONST: [[W4:_S[0-9]+]] : u32 = pool_{{[0-9]+}}[(u32(260))/4]
// CONST-NEXT: [[F:_S[0-9]+]] : f32 = bitcast<f32>([[W4]])
// CONST: [[W5:_S[0-9]+]] : u32 = pool_{{[0-9]+}}[(u32(264))/4]
// CONST-NEXT: [[H5:_S[0-9]+]] : vec2<f16> = bitcast<vec2<f16>>(([[W5]] >> (u32(0))))
// CONST: [[W6:_S[0-9]+]] : u32 = pool_{{[0-9]+}}[(u32(264))/4]
// CONST: f32([[H0]][i32(0)]), f32([[H1]][i32(0)]), f32([[H2]][i32(0)]) + f32([[H3]][i32(0)]), f32([[H5]][i32(0)]) + f32(bitcast<vec2<f16>>(([[W6]] >> (u32(16))))[i32(0)]) + [[F]])

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
