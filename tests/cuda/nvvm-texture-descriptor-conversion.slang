//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3 -Xslang -emit-cuda-via-nvvm

// The two descriptors exercise the material's floating-point and integer texture families.
//TEST_INPUT: Texture2D(size=4, content=one):name inputTexture
DescriptorHandle<Texture2D<float4>> inputTexture;
//TEST_INPUT: Texture2D(format=R32Uint, size=4, content=one):name integerTexture
DescriptorHandle<Texture2D<uint>> integerTexture;
//TEST_INPUT: ubuffer(data=[324508639 99 99 99 99 99 99 99 99 3735928559], stride=4):out,name outputBuffer
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

// Each new direction crosses a helper boundary and is compared with the existing UInt64 API.
[noinline]
uint2 readHandleWords<T : ITexelElement>(DescriptorHandle<Texture2D<T>> handle)
{
    return uint2(handle);
}

[noinline]
DescriptorHandle<Texture2D<T>> makeHandleWords<T : ITexelElement>(uint2 words)
{
    return DescriptorHandle<Texture2D<T>>(words);
}

uint wordTransportFailures(uint64_t bits, uint2 expectedWords, int2 coordinate)
{
    __target_switch
    {
    case nvvm:
        uint failures = readHandle(makeHandleWords<float4>(expectedWords)) == bits ? 0u : 8u;
        if (any(readHandleWords(makeHandle(bits)) != expectedWords))
            failures |= 16;
        // Only actual bound texture objects are dereferenced. Sentinel bits stay transport-only.
        uint64_t floatHandle = readHandle(makeHandleWords<float4>(readHandleWords(inputTexture)));
        uint64_t integerHandle = readHandle(makeHandleWords<uint>(readHandleWords(integerTexture)));
        if (any(loadTexture(floatHandle, coordinate) != float4(1.0)))
            failures |= 32;
        if (loadIntegerTexture(integerHandle, coordinate) != 1)
            failures |= 64;
        return failures;
    case cuda:
    default:
        return 0;
    }
}

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Boundary payloads are transported only. Texture loads below use real bound handles.
    uint64_t bits =
        tid.x == 0 ? uint64_t(0) :
        tid.x == 1 ? uint64_t(0x8000000000000001UL) :
        tid.x == 2 ? uint64_t(0xffffffffffffffffUL) : uint64_t(0x123456789abcdef0UL);
    uint2 expectedWords =
        tid.x == 0 ? uint2(0, 0) :
        tid.x == 1 ? uint2(0x00000001u, 0x80000000u) :
        tid.x == 2 ? uint2(0xffffffffu, 0xffffffffu) : uint2(0x9abcdef0u, 0x12345678u);
    uint failures = readHandle(makeHandle(bits)) == bits ? 0 : 1;
    int2 coordinate = int2((tid.x & 1) * 3, (tid.x >> 1) * 3);
    if (any(loadTexture(readHandle(inputTexture), coordinate) != float4(1.0)))
        failures |= 2;
    if (loadIntegerTexture(readHandle(integerTexture), coordinate) != 1)
        failures |= 4;
    failures |= wordTransportFailures(bits, expectedWords, coordinate);
    outputBuffer[1 + 2 * tid.x] = failures;
    outputBuffer[2 + 2 * tid.x] = 4096 + tid.x;
}
//CHECK: {{^}}324508639{{$}}
//CHECK-NEXT: {{^}}0{{$}}
//CHECK-NEXT: {{^}}4096{{$}}
//CHECK-NEXT: {{^}}0{{$}}
//CHECK-NEXT: {{^}}4097{{$}}
//CHECK-NEXT: {{^}}0{{$}}
//CHECK-NEXT: {{^}}4098{{$}}
//CHECK-NEXT: {{^}}0{{$}}
//CHECK-NEXT: {{^}}4099{{$}}
//CHECK-NEXT: {{^}}3735928559{{$}}
