//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -shaderobj -render-feature half
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -render-feature half

// WGSL's `bitcast` converts directly between `vec2<f16>` and a 32-bit scalar (`u32`, `i32`,
// `f32`), and between `vec4<f16>` and a `vec2` of 32-bit values. `bit_cast` of those shapes is
// emitted as a WGSL `bitcast` in both directions, not lowered through 16-bit integers, which
// WGSL does not have.

// CHECK-NOT: {{error\[E[0-9]+\]}}
// CHECK-DAG: bitcast<vec2<f16>>(
// CHECK-DAG: bitcast<vec4<f16>>(
// CHECK-DAG: bitcast<u32>(vec2<f16>(
// CHECK-DAG: bitcast<i32>(vec2<f16>(
// CHECK-DAG: bitcast<f32>(vec2<f16>(
// CHECK-DAG: bitcast<vec2<u32>>(
// CHECK-NOT: {{error\[E[0-9]+\]}}
// TINT: OpEntryPoint

// The runs check exact bits. Each `half` that comes out of a bitcast is widened to `float`, which
// is exact, and written as its `float` bits, so a swapped element or a wrong half-word shows up
// as a different value. The other direction writes the bitcast's 32-bit result as it is. The
// Vulkan run is a reference for the same expected values.
//
// All values are normal and nonzero, and so is every 32-bit word read as `float`, because WGSL
// lets an implementation flush subnormals to zero and ignore the sign of zero.
//
// Input words, as (element 0 = low 16 bits, element 1 = high 16 bits):
//   0x40003C00 = (1.0, 2.0)                      0xC2003800 = (0.5, -3.0)
//   0x04017BFF = (65504.0, 2^-14 * (1 + 2^-10))  0xB0003555 = (0.333251953125, -0.125)

//TEST_INPUT:ubuffer(data=[0x40003C00 0xC2003800 0x04017BFF 0xB0003555], stride=16):name=words
RWStructuredBuffer<uint4> words;

//TEST_INPUT:ubuffer(data=[0x3F800000 0x40000000 0x3F000000 0xC0400000 0x477FE000 0x38802000 0x3EAAA000 0xBE000000], stride=4):name=floats
RWStructuredBuffer<float> floats;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// 32-bit -> half2 (from u32, i32 and f32) and uint2 -> half4.
// BUF: {{^}}3F800000{{$}}
// BUF-NEXT: {{^}}40000000{{$}}
// BUF-NEXT: {{^}}3F000000{{$}}
// BUF-NEXT: {{^}}C0400000{{$}}
// BUF-NEXT: {{^}}477FE000{{$}}
// BUF-NEXT: {{^}}38802000{{$}}
// BUF-NEXT: {{^}}477FE000{{$}}
// BUF-NEXT: {{^}}38802000{{$}}
// BUF-NEXT: {{^}}3EAAA000{{$}}
// BUF-NEXT: {{^}}BE000000{{$}}
// half2 -> u32, i32 and f32, and half4 -> uint2.
// BUF-NEXT: {{^}}40003C00{{$}}
// BUF-NEXT: {{^}}C2003800{{$}}
// BUF-NEXT: {{^}}C2003C00{{$}}
// BUF-NEXT: {{^}}4017BFF{{$}}
// BUF-NEXT: {{^}}B0003555{{$}}

void writeHalf2(uint index, half2 h)
{
    outputBuffer[index] = asuint(float(h.x));
    outputBuffer[index + 1] = asuint(float(h.y));
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    const uint4 w = words[dispatchThreadID.x];

    writeHalf2(0, bit_cast<half2>(w.x));
    writeHalf2(2, bit_cast<half2>(int(w.y)));
    writeHalf2(4, bit_cast<half2>(asfloat(w.z)));
    const half4 fromUInt2 = bit_cast<half4>(w.zw);
    writeHalf2(6, fromUInt2.xy);
    writeHalf2(8, fromUInt2.zw);

    // Halves converted from `float`, so these bitcasts cannot fold with the ones above.
    const half4 h = half4(half(floats[0]), half(floats[1]), half(floats[2]), half(floats[3]));
    const half4 g = half4(half(floats[4]), half(floats[5]), half(floats[6]), half(floats[7]));
    outputBuffer[10] = bit_cast<uint>(h.xy);
    outputBuffer[11] = uint(bit_cast<int>(h.zw));
    outputBuffer[12] = asuint(bit_cast<float>(h.xw));
    const uint2 fromHalf4 = bit_cast<uint2>(g);
    outputBuffer[13] = fromHalf4.x;
    outputBuffer[14] = fromHalf4.y;
}
