// DescriptorHandle<T> fields in interface implementations are marshalled
// into AnyValue as uint2 (non-bindless) or native handle (bindless).
// This test verifies that a DescriptorHandle survives AnyValue packing
// through dynamic dispatch and remains usable for buffer access.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -render-feature bindless -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type -profile sm_6_6 -render-feature bindless
// Metal disabled: DescriptorHandle as_type cast bug (https://github.com/shader-slang/slang/issues/10477)

interface IDataSource
{
    float getValue(int index);
}

struct BufferSource : IDataSource
{
    StructuredBuffer<float>.Handle dataHandle;
    float offset;

    float getValue(int index)
    {
        return dataHandle[index] + offset;
    }
}

struct ConstSource : IDataSource
{
    float value;

    float getValue(int index)
    {
        return value;
    }
}

IDataSource makeSource(
    int kind,
    StructuredBuffer<float>.Handle handle)
{
    if (kind == 0)
    {
        BufferSource bs;
        bs.dataHandle = handle;
        bs.offset = 100.0;
        return bs;
    }
    ConstSource cs;
    cs.value = 42.0;
    return cs;
}

//TEST_INPUT:ubuffer(data=[10.0 20.0 30.0 40.0], stride=4):name=dataBuffer
uniform StructuredBuffer<float>.Handle dataBuffer;

//TEST_INPUT:ubuffer(data=[0 1], stride=4):name=kindBuffer
StructuredBuffer<int> kindBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IDataSource bufSrc = makeSource(kindBuffer[0], dataBuffer);

    // CHECK: 110.0
    outputBuffer[0] = bufSrc.getValue(0);
    // CHECK: 120.0
    outputBuffer[1] = bufSrc.getValue(1);

    IDataSource constSrc = makeSource(kindBuffer[1], dataBuffer);

    // CHECK: 42.0
    outputBuffer[2] = constSrc.getValue(0);
    // CHECK: 42.0
    outputBuffer[3] = constSrc.getValue(1);
}
