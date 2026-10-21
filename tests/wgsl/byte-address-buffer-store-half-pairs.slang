//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -shaderobj -render-feature half
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -render-feature half

// WGSL byte-address buffers are arrays of 32-bit words and WGSL has no 16-bit integers. A
// `half2` or `half4` stored at an offset known to be a multiple of 4 (a constant offset, an
// explicit alignment of 4, or `StoreAligned`) fills whole words, so each pair of halves is
// stored as one word, `bitcast<u32>(vec2<f16>(...))`, with no read-modify-write of the word.

// CHECK-NOT: {{error\[E[0-9]+\]}}
// CHECK: fn computeMain
// CHECK-NOT: & (u32(4294901760))
// CHECK-NOT: & (u32(65535))
// CHECK: pool_{{[0-9]+}}[(u32(0))/4] = bitcast<u32>(
// CHECK: pool_{{[0-9]+}}[(u32(4))/4] = bitcast<u32>(
// CHECK: pool_{{[0-9]+}}[(u32(8))/4] = bitcast<u32>(
// CHECK: pool_{{[0-9]+}}[({{.*}})/4] = bitcast<u32>(
// CHECK: pool_{{[0-9]+}}[(u32(20))/4] = bitcast<u32>(
// CHECK: pool_{{[0-9]+}}[({{.*}})/4] = bitcast<u32>(
// CHECK-NOT: & (u32(4294901760))
// CHECK-NOT: & (u32(65535))
// TINT: OpEntryPoint

// Words, as (half at the lower byte offset, half at the higher one). All values are normal and
// nonzero, so a WGSL implementation that flushes subnormals or ignores the sign of zero gives
// the same bits.
//   word 0: 0x40003C00 = (1.0, 2.0)
//   word 1: 0xC2003800 = (0.5, -3.0)
//   word 2: 0x04017BFF = (65504.0, 2^-14 * (1 + 2^-10))
//   word 3: 0xB0003555 = (0.333251953125, -0.125)
//   word 4: 0x40E00000 = 7.0f
//   word 5: 0x38004800 = (8.0, 0.5)
//   word 6: 0x40003C00 = (1.0, 2.0), through `StoreAligned`

//TEST_INPUT:ubuffer(data=[0x3F800000 0x40000000 0x3F000000 0xC0400000 0x477FE000 0x38802000 0x3EAAA000 0xBE000000 0x41000000], stride=4):name=floats
RWStructuredBuffer<float> floats;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=pool
RWByteAddressBuffer pool;

struct Record
{
    float f;
    half2 h;
};

// BUF: {{^}}40003C00{{$}}
// BUF-NEXT: {{^}}C2003800{{$}}
// BUF-NEXT: {{^}}4017BFF{{$}}
// BUF-NEXT: {{^}}B0003555{{$}}
// BUF-NEXT: {{^}}40E00000{{$}}
// BUF-NEXT: {{^}}38004800{{$}}
// BUF-NEXT: {{^}}40003C00{{$}}
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    const uint i = dispatchThreadID.x;

    // Constant offsets that are multiples of 4.
    pool.Store<half2>(0, half2(half(floats[0]), half(floats[1])));
    pool.Store<half4>(
        4,
        half4(half(floats[2]), half(floats[3]), half(floats[4]), half(floats[5])));

    // A run-time offset with an explicit alignment of 4.
    pool.Store<half2>(i * 4 + 12, half2(half(floats[6]), half(floats[7])), 4);

    // A struct at a constant offset: its `half2` field is at byte 20.
    Record r;
    r.f = 7.0;
    r.h = half2(half(floats[8]), half(floats[2]));
    pool.Store<Record>(16, r);

    // `StoreAligned` promises the natural alignment of `half2`, which is 4.
    pool.StoreAligned<half2>(i * 4 + 24, half2(half(floats[0]), half(floats[1])));
}
