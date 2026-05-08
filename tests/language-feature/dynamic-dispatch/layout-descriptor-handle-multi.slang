// Multiple DescriptorHandle fields in a single interface implementation test
// that AnyValue packing correctly handles per-handle offsets. Each handle
// occupies 8 bytes (uint2) on non-bindless targets and must be independently
// usable after unpacking through dynamic dispatch.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -render-feature bindless -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type -profile sm_6_6 -render-feature bindless
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-mtl -compute -shaderobj -output-using-type

interface IMultiSource
{
    float getA(int index);
    float getB(int index);
}

struct DualBufferSource : IMultiSource
{
    StructuredBuffer<float>.Handle bufA;
    StructuredBuffer<float>.Handle bufB;

    float getA(int index) { return bufA[index]; }
    float getB(int index) { return bufB[index]; }
}

struct ConstSource : IMultiSource
{
    float valueA;
    float valueB;

    float getA(int index) { return valueA; }
    float getB(int index) { return valueB; }
}

IMultiSource makeMultiSource(
    int kind,
    StructuredBuffer<float>.Handle a,
    StructuredBuffer<float>.Handle b)
{
    if (kind == 0)
    {
        DualBufferSource dbs;
        dbs.bufA = a;
        dbs.bufB = b;
        return dbs;
    }
    ConstSource cs;
    cs.valueA = 99.0;
    cs.valueB = 77.0;
    return cs;
}

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0], stride=4):name=bufferA
uniform StructuredBuffer<float>.Handle bufferA;

//TEST_INPUT:ubuffer(data=[10.0 20.0 30.0], stride=4):name=bufferB
uniform StructuredBuffer<float>.Handle bufferB;

//TEST_INPUT:ubuffer(data=[0 1], stride=4):name=kindBuffer
StructuredBuffer<int> kindBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IMultiSource dualSrc = makeMultiSource(kindBuffer[0], bufferA, bufferB);

    // CHECK: 1.0
    outputBuffer[0] = dualSrc.getA(0);
    // CHECK: 20.0
    outputBuffer[1] = dualSrc.getB(1);
    // CHECK: 3.0
    outputBuffer[2] = dualSrc.getA(2);

    IMultiSource constSrc = makeMultiSource(kindBuffer[1], bufferA, bufferB);

    // CHECK: 99.0
    outputBuffer[3] = constSrc.getA(0);
    // CHECK: 77.0
    outputBuffer[4] = constSrc.getB(0);
    // CHECK: 99.0
    outputBuffer[5] = constSrc.getA(1);
}
