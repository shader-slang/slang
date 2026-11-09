// Load with a texel offset must read the same texel as Load at the offset coordinate (and the
// offset must matter: the neighbouring texel differs in the gradient texture).
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -wgpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -vk -shaderobj -output-using-type

//TEST_INPUT: Texture2D(size=8, content=gradient, mipMaps=1):name t2D
Texture2D<float4> t2D;
//TEST_INPUT: Texture2D(size=8, content=gradient, mipMaps=1, arrayLength=2):name t2DArray
Texture2DArray<float4> t2DArray;
//TEST_INPUT: Texture3D(size=8, content=gradient, mipMaps=1):name t3D
Texture3D<float4> t3D;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int2 p = int2(3, 4);
    bool same2D = all(t2D.Load(int3(p, 0), int2(2, -1)) == t2D.Load(int3(p + int2(2, -1), 0)));
    bool differs2D = any(t2D.Load(int3(p, 0), int2(2, -1)) != t2D.Load(int3(p, 0)));
    bool sameArray =
        all(t2DArray.Load(int4(p, 1, 0), int2(-1, 1)) == t2DArray.Load(int4(p + int2(-1, 1), 1, 0)));
    bool same3D =
        all(t3D.Load(int4(p, 2, 0), int3(1, 0, -1)) == t3D.Load(int4(p + int2(1, 0), 1, 0)));

    // CHECK: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    outputBuffer[0] = same2D ? 1 : 0;
    outputBuffer[1] = differs2D ? 1 : 0;
    outputBuffer[2] = sameArray ? 1 : 0;
    outputBuffer[3] = same3D ? 1 : 0;
}
