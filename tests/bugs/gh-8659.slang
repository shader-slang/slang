//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly

//TEST_INPUT: set ptr = ubuffer(data=[0 0 0 0], stride=4)
uniform uint* ptr;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    let indices = (uint*)&ptr[0];
    *indices = 100;
    // CHECK: 100
    outputBuffer[0] = ptr[0];
}