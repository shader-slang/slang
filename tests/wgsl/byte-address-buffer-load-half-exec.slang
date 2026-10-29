//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -shaderobj -render-feature half
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -shaderobj -render-feature half -xslang -DRWBAB
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -render-feature half

// Runs the WGSL lowering of byte-address-buffer `half` loads (see
// byte-address-buffer-load-half.slang), which loads the containing 32-bit word, shifts it and
// bit-casts it to `vec2<f16>`. It loads `half`, `half2`, `half4` and a struct with `half` fields,
// at 4-byte aligned offsets and at offsets that are only 2-byte aligned, so both half-words of
// a word are read, and a `half2` or `half4` can start in the middle of a word.
//
// Every loaded `half` is widened to `float`, which is exact, and written as its `float` bits, so a
// swapped half-word or a wrong shift shows up as a different value. The vector loads use an offset
// read from the buffer (word 6 is 0), so they take the dynamic-offset path. The Vulkan run is a
// reference for the same expected values.
//
// All loaded values are normal and nonzero, because WGSL lets an implementation flush subnormals
// to zero and ignore the sign of zero.
//
// Words, as (half at the lower byte offset, half at the higher one):
//   byte  0: 0x40003C00 = (1.0, 2.0)
//   byte  4: 0xC2003800 = (0.5, -3.0)
//   byte  8: 0x04017BFF = (65504.0, 2^-14 * (1 + 2^-10))
//   byte 12: 0xB0003555 = (0.333251953125, -0.125)
//   byte 16: 0x40E00000 = 7.0f
//   byte 20: 0x38004800 = (8.0, 0.5)
//   byte 24: 0

//TEST_INPUT:ubuffer(data=[0x40003C00 0xC2003800 0x04017BFF 0xB0003555 0x40E00000 0x38004800 0], stride=4):name=pool
#if defined(RWBAB)
RWByteAddressBuffer pool;
#else
ByteAddressBuffer pool;
#endif

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

struct Record
{
    half a;
    half b;
    float c;
    half2 d;
};

void writeHalf(uint index, half h)
{
    outputBuffer[index] = asuint(float(h));
}

void writeHalf2(uint index, half2 h)
{
    writeHalf(index, h.x);
    writeHalf(index + 1, h.y);
}

void writeHalf4(uint index, half4 h)
{
    writeHalf2(index, h.xy);
    writeHalf2(index + 2, h.zw);
}

// `half` at bytes 0, 2, ..., 14 (thread i reads byte 2 * i).
// BUF: {{^}}3F800000{{$}}
// BUF-NEXT: {{^}}40000000{{$}}
// BUF-NEXT: {{^}}3F000000{{$}}
// BUF-NEXT: {{^}}C0400000{{$}}
// BUF-NEXT: {{^}}477FE000{{$}}
// BUF-NEXT: {{^}}38802000{{$}}
// BUF-NEXT: {{^}}3EAAA000{{$}}
// BUF-NEXT: {{^}}BE000000{{$}}
// `half2` at byte 4 (aligned), then at byte 2 (across two words).
// BUF-NEXT: {{^}}3F000000{{$}}
// BUF-NEXT: {{^}}C0400000{{$}}
// BUF-NEXT: {{^}}40000000{{$}}
// BUF-NEXT: {{^}}3F000000{{$}}
// `half4` at byte 8 (aligned), then at byte 6 (across three words).
// BUF-NEXT: {{^}}477FE000{{$}}
// BUF-NEXT: {{^}}38802000{{$}}
// BUF-NEXT: {{^}}3EAAA000{{$}}
// BUF-NEXT: {{^}}BE000000{{$}}
// BUF-NEXT: {{^}}C0400000{{$}}
// BUF-NEXT: {{^}}477FE000{{$}}
// BUF-NEXT: {{^}}38802000{{$}}
// BUF-NEXT: {{^}}3EAAA000{{$}}
// `Record` at byte 12: a, b, c (the `float` bits as loaded), d.x, d.y.
// BUF-NEXT: {{^}}3EAAA000{{$}}
// BUF-NEXT: {{^}}BE000000{{$}}
// BUF-NEXT: {{^}}40E00000{{$}}
// BUF-NEXT: {{^}}41000000{{$}}
// BUF-NEXT: {{^}}3F000000{{$}}
[numthreads(8, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    const uint i = dispatchThreadID.x;
    writeHalf(i, pool.Load<half>(i * 2));
    if (i == 0)
    {
        const uint zero = pool.Load(24);
        writeHalf2(8, pool.Load<half2>(zero + 4));
        writeHalf2(10, pool.Load<half2>(zero + 2));
        writeHalf4(12, pool.Load<half4>(zero + 8));
        writeHalf4(16, pool.Load<half4>(zero + 6));

        const Record r = pool.Load<Record>(12);
        writeHalf2(20, half2(r.a, r.b));
        outputBuffer[22] = asuint(r.c);
        writeHalf2(23, r.d);
    }
}
