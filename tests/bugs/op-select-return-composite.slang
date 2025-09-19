//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-d3d12 -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -profile sm_6_0

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct CompositeType
{
    __init(int dataIn)
    {
        data1 = dataIn;
        data2 = dataIn;
    }
    int data1;
    float data2;
}

[numthreads(1,1,1)]
void computeMain(){

    CompositeType composite = CompositeType(-1); 
    if (outputBuffer[0] == 0)
    {
        composite = outputBuffer[1] > -1 ? CompositeType(1) : CompositeType(-1);
    }
    outputBuffer[2] = composite.data1;
    outputBuffer[3] = (int)composite.data2;
}

//BUF: 0
//BUF: 0
//BUF: 1
//BUF: 1
