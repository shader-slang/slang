// Texture.Load of a three-component texel type returns the first three channels of the texel,
// in order. On CUDA the three-component fetch reads four channels and keeps the first three; the
// gradient texture gives every channel of every texel a distinct value, so a reordered or
// shifted component would change the output. The CPU lane checks the same expected values.
//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj -output-using-type

//TEST_INPUT: Texture2D(size=4, content=gradient, format=RGBA32Float):name t2
Texture2D<float3> t2;
//TEST_INPUT: Texture2D(size=4, content=gradient, format=RGBA32Float, arrayLength=2):name t2a
Texture2DArray<float3> t2a;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float3 a = t2.Load(int3(1, 2, 0));
    float3 b = t2a.Load(int4(3, 1, 1, 0));
    outputBuffer[0] = a.x;
    outputBuffer[1] = a.y;
    outputBuffer[2] = a.z;
    outputBuffer[3] = b.x;
    outputBuffer[4] = b.y;
    outputBuffer[5] = b.z;
}
