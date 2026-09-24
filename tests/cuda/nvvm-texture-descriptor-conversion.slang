//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3 -Xslang -emit-cuda-via-nvvm

// The two descriptors exercise the material's floating-point and integer texture families.
//TEST_INPUT: Texture2D(size=4, content=one):name inputTexture
DescriptorHandle<Texture2D<float4>> inputTexture;
//TEST_INPUT: Texture2D(format=R32Uint, size=4, content=one):name integerTexture
DescriptorHandle<Texture2D<uint>> integerTexture;
//TEST_INPUT: ubuffer(data=[99 99 99 99], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// Keep the conversions across helper boundaries so Slang cannot fold the round trip away.
[noinline]
uint64_t readHandle<T : ITexelElement>(DescriptorHandle<Texture2D<T>> handle)
{
    return uint64_t(handle);
}

[noinline]
DescriptorHandle<Texture2D<float4>> makeHandle(uint64_t bits)
{
    return DescriptorHandle<Texture2D<float4>>(bits);
}

[noinline]
float4 loadTexture(uint64_t handle, int2 coord)
{
    Texture2D<float4> texture = DescriptorHandle<Texture2D<float4>>(handle);
    return texture.Load(int3(coord, 0));
}

[noinline]
uint loadIntegerTexture(uint64_t handle, int2 coord)
{
    Texture2D<uint> texture = DescriptorHandle<Texture2D<uint>>(handle);
    return texture.Load(int3(coord, 0));
}

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Boundary payloads are transported only. Texture loads below use real bound handles.
    uint64_t bits =
        tid.x == 0 ? uint64_t(0) :
        tid.x == 1 ? uint64_t(0x8000000000000001UL) :
        tid.x == 2 ? uint64_t(0xffffffffffffffffUL) : uint64_t(0x123456789abcdef0UL);
    uint failures = readHandle(makeHandle(bits)) == bits ? 0 : 1;
    int2 coordinate = int2((tid.x & 1) * 3, (tid.x >> 1) * 3);
    if (any(loadTexture(readHandle(inputTexture), coordinate) != float4(1.0)))
        failures |= 2;
    if (loadIntegerTexture(readHandle(integerTexture), coordinate) != 1)
        failures |= 4;
    outputBuffer[tid.x] = failures;
}
//CHECK: 0
//CHECK: 0
//CHECK: 0
//CHECK: 0
