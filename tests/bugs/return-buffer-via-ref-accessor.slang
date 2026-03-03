//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -vk -compute -shaderobj -profile sm_6_6
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -dx12 -compute -shaderobj -profile sm_6_6

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: set globalBuffers = new StructuredBuffer<RWStructuredBuffer<int>.Handle>[ubuffer_handle(data=[1 1 1 1], stride=4), ubuffer_handle(data=[1 1 1 1], stride=4)];
StructuredBuffer<RWStructuredBuffer<int>.Handle> globalBuffers;

struct BufferWrapper
{
    uint index;

    __subscript(uint i)->int
    {
        ref
        {
            return globalBuffers[index][i];
        }
    }
};

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    //CHECK: 3
    BufferWrapper data;
    data.index = 1;
    InterlockedAdd(data[2], 2);
    outputBuffer[1] = data[2];
}