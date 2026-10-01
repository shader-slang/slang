//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Expected counts come from the declared bound arrays. CUDA C++ currently returns zero, so it
// is not a correctness oracle for this query. Singleton input binding is a separate host issue.
//TEST_INPUT: Texture2D(format=R32Uint, size=1, arrayLength=2, content=one):name twoLayers
DescriptorHandle<Texture2DArray<uint>> twoLayers;
//TEST_INPUT: Texture2D(format=RG32Sint, size=4, arrayLength=3, content=one):name threeLayers
Texture2DArray<int2> threeLayers;
//TEST_INPUT: Texture2D(format=RGBA32Float, size=8, arrayLength=5, content=one):name fiveLayers
Texture2DArray<float4> fiveLayers;
//TEST_INPUT: Texture1D(format=R32Uint, size=1, arrayLength=2, content=one):name oneDimensionalTwoLayers
DescriptorHandle<Texture1DArray<uint>> oneDimensionalTwoLayers;
//TEST_INPUT: Texture1D(format=RG32Sint, size=4, arrayLength=3, content=one):name oneDimensionalThreeLayers
Texture1DArray<int2> oneDimensionalThreeLayers;
//TEST_INPUT: Texture1D(format=RGBA32Float, size=8, arrayLength=5, content=one):name oneDimensionalFiveLayers
Texture1DArray<float4> oneDimensionalFiveLayers;
//TEST_INPUT: ubuffer(data=[324508639 99 99 99 99 99 99 99 3735928559], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[noinline]
uint checkDimensions<T : ITexelElement>(Texture2DArray<T> texture, int width, int layers)
{
    int3 signedSize = int3(-1);
    uint3 unsignedSize = uint3(0xffffffff);
    float3 floatSize = float3(-1.0f);
    texture.GetDimensions(signedSize.x, signedSize.y, signedSize.z);
    texture.GetDimensions(unsignedSize.x, unsignedSize.y, unsignedSize.z);
    texture.GetDimensions(floatSize.x, floatSize.y, floatSize.z);
    int3 expected = int3(width, width, layers);
    return any(signedSize != expected) || any(unsignedSize != uint3(expected))
        || any(floatSize != float3(expected)) ? 1 : 0;
}

[noinline]
uint checkHandle(uint64_t handle)
{
    Texture2DArray<uint> texture = DescriptorHandle<Texture2DArray<uint>>(handle);
    return checkDimensions(texture, 1, 2);
}

[noinline]
uint checkDimensions<T : ITexelElement>(Texture1DArray<T> texture, int width, int layers)
{
    int2 signedSize = int2(-1);
    uint2 unsignedSize = uint2(0xffffffff);
    float2 floatSize = float2(-1.0f);
    texture.GetDimensions(signedSize.x, signedSize.y);
    texture.GetDimensions(unsignedSize.x, unsignedSize.y);
    texture.GetDimensions(floatSize.x, floatSize.y);
    int2 expected = int2(width, layers);
    return any(signedSize != expected) || any(unsignedSize != uint2(expected))
        || any(floatSize != float2(expected)) ? 1 : 0;
}

[noinline]
uint checkOneDimensionalHandle(uint64_t handle)
{
    Texture1DArray<uint> texture = DescriptorHandle<Texture1DArray<uint>>(handle);
    return checkDimensions(texture, 1, 2);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[1] = checkHandle(uint64_t(twoLayers));
    outputBuffer[2] = checkDimensions(threeLayers, 4, 3);
    outputBuffer[3] = checkDimensions(fiveLayers, 8, 5);
    outputBuffer[4] = checkOneDimensionalHandle(uint64_t(oneDimensionalTwoLayers));
    outputBuffer[5] = checkDimensions(oneDimensionalThreeLayers, 4, 3);
    outputBuffer[6] = checkDimensions(oneDimensionalFiveLayers, 8, 5);
    outputBuffer[7] = 4096;
}
//CHECK: type: uint32_t
//CHECK-NEXT: {{^}}324508639{{$}}
//CHECK-NEXT: {{^}}0{{$}}
//CHECK-NEXT: {{^}}0{{$}}
//CHECK-NEXT: {{^}}0{{$}}
//CHECK-NEXT: {{^}}0{{$}}
//CHECK-NEXT: {{^}}0{{$}}
//CHECK-NEXT: {{^}}0{{$}}
//CHECK-NEXT: {{^}}4096{{$}}
//CHECK-NEXT: {{^}}3735928559{{$}}
