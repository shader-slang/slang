//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Dimension queries depend on bound resource geometry, including the one-texel boundary.
//TEST_INPUT: Texture2D(format=R32Float, size=1, content=one):name textureFloat
Texture2D<float> textureFloat;
//TEST_INPUT: Texture2D(format=RG32Float, size=4, content=one):name textureFloat2
Texture2D<float2> textureFloat2;
//TEST_INPUT: Texture2D(format=RGBA32Float, size=8, content=one):name textureFloat4
Texture2D<float4> textureFloat4;
//TEST_INPUT: Texture2D(format=R32Sint, size=1, content=one):name textureInt
Texture2D<int> textureInt;
//TEST_INPUT: Texture2D(format=RG32Sint, size=4, content=one):name textureInt2
Texture2D<int2> textureInt2;
//TEST_INPUT: Texture2D(format=RGBA32Sint, size=8, content=one):name textureInt4
Texture2D<int4> textureInt4;
//TEST_INPUT: Texture2D(format=R32Uint, size=1, content=one):name textureUint
DescriptorHandle<Texture2D<uint>> textureUint;
//TEST_INPUT: Texture2D(format=RG32Uint, size=4, content=one):name textureUint2
Texture2D<uint2> textureUint2;
//TEST_INPUT: Texture2D(format=RGBA32Uint, size=8, content=one):name textureUint4
Texture2D<uint4> textureUint4;
//TEST_INPUT: Texture1D(format=R32Uint, size=1, content=one):name oneDimensional
Texture1D<uint> oneDimensional;
//TEST_INPUT: Texture3D(format=RGBA32Sint, size=2, content=one):name threeDimensional
Texture3D<int4> threeDimensional;
//TEST_INPUT: TextureCube(format=RG32Uint, size=4, content=one):name cubeTexture
TextureCube<uint2> cubeTexture;
//TEST_INPUT: Texture2D(format=RGBA32Float, size=8, arrayLength=3, content=one):name arrayTexture
Texture2DArray<float4> arrayTexture;
//TEST_INPUT: TextureCube(format=RG32Sint, size=4, arrayLength=1, content=one):name cubeArrayTexture
TextureCubeArray<int2> cubeArrayTexture;
//TEST_INPUT: ubuffer(data=[99 99 99 99 99 99 99 99 99 99 99 99 99 99], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[noinline]
uint checkDimensions<T : ITexelElement>(Texture2D<T> texture, int expected)
{
    int2 signedSize = int2(-1);
    uint2 unsignedSize = uint2(0xffffffff);
    texture.GetDimensions(signedSize.x, signedSize.y);
    texture.GetDimensions(unsignedSize.x, unsignedSize.y);
    return any(signedSize != int2(expected)) || any(unsignedSize != uint2(expected)) ? 1 : 0;
}

[noinline]
uint checkIntegerHandle(uint64_t handle)
{
    // This is the material's descriptor-to-texture and signed dimension helper boundary.
    Texture2D<uint> texture = DescriptorHandle<Texture2D<uint>>(handle);
    return checkDimensions(texture, 1);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = checkDimensions(textureFloat, 1);
    outputBuffer[1] = checkDimensions(textureFloat2, 4);
    outputBuffer[2] = checkDimensions(textureFloat4, 8);
    outputBuffer[3] = checkDimensions(textureInt, 1);
    outputBuffer[4] = checkDimensions(textureInt2, 4);
    outputBuffer[5] = checkDimensions(textureInt4, 8);
    outputBuffer[6] = checkIntegerHandle(uint64_t(textureUint));
    outputBuffer[7] = checkDimensions(textureUint2, 4);
    outputBuffer[8] = checkDimensions(textureUint4, 8);
    int width = -1;
    oneDimensional.GetDimensions(width);
    outputBuffer[9] = width != 1 ? 1 : 0;
    int3 volumeSize = int3(-1);
    threeDimensional.GetDimensions(volumeSize.x, volumeSize.y, volumeSize.z);
    outputBuffer[10] = any(volumeSize != int3(2)) ? 1 : 0;
    int2 cubeSize = int2(-1);
    cubeTexture.GetDimensions(cubeSize.x, cubeSize.y);
    outputBuffer[11] = any(cubeSize != int2(4)) ? 1 : 0;
    int3 arraySize = int3(-1);
    arrayTexture.GetDimensions(arraySize.x, arraySize.y, arraySize.z);
    // CUDA's prelude explicitly returns zero for unavailable array-size queries.
    outputBuffer[12] = any(arraySize != int3(8, 8, 0)) ? 1 : 0;
    cubeArrayTexture.GetDimensions(arraySize.x, arraySize.y, arraySize.z);
    outputBuffer[13] = any(arraySize != int3(4, 4, 0)) ? 1 : 0;
}
//CHECK: 0
//CHECK: 0
//CHECK: 0
//CHECK: 0
//CHECK: 0
//CHECK: 0
//CHECK: 0
//CHECK: 0
//CHECK: 0
//CHECK: 0
//CHECK: 0
//CHECK: 0
//CHECK: 0
//CHECK: 0
