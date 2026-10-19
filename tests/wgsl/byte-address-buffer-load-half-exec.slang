//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -shaderobj -render-feature half
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -shaderobj -render-feature half -xslang -DRWBAB
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -render-feature half

// Runs the WGSL lowering of byte-address-buffer `half` loads (see
// byte-address-buffer-load-half.slang), which loads the containing 32-bit word, shifts it and
// bit-casts it to `vec2<f16>`. The words hold known half bit patterns:
// - 0x40003C00 0x44004200: 1.0, 2.0, 3.0, 4.0, read at dynamic offsets (thread i reads byte 2*i);
// - a `Record` at byte 8: a = 5.0, b = 6.0 (0x46004500), c = 7.0f (0x40E00000), and
//   d = (8.0, 0.5) (0x38004800).
// Each value is written as uint(2 * value), so a swapped half or a wrong shift shows up.
// The Vulkan run is a reference for the same expected values.

//TEST_INPUT:ubuffer(data=[1073757184 1140867584 1174422784 1088421888 939542528], stride=4):name pool
#if defined(RWBAB)
RWByteAddressBuffer pool;
#else
ByteAddressBuffer pool;
#endif

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

struct Record
{
    half a;
    half b;
    float c;
    half2 d;
};

// BUF: 2
// BUF-NEXT: 4
// BUF-NEXT: 6
// BUF-NEXT: 8
// BUF-NEXT: A
// BUF-NEXT: C
// BUF-NEXT: E
// BUF-NEXT: 10
// BUF-NEXT: 1
[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    const uint i = dispatchThreadID.x;
    outputBuffer[i] = uint(2.0 * float(pool.Load<half>(i * 2)));
    if (i == 0)
    {
        const Record r = pool.Load<Record>(8 + i);
        outputBuffer[4] = uint(2.0 * float(r.a));
        outputBuffer[5] = uint(2.0 * float(r.b));
        outputBuffer[6] = uint(2.0 * r.c);
        outputBuffer[7] = uint(2.0 * float(r.d.x));
        outputBuffer[8] = uint(2.0 * float(r.d.y));
    }
}
