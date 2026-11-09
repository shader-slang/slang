// Dereference a pointer to a mutable structured-buffer element with *, ->, and .
// (shader-slang/slang#12581).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct Pair
{
    int x;
    int y;
}

//TEST_INPUT:ubuffer(data=[10 20 0 0], stride=8):name=dataBuffer
RWStructuredBuffer<Pair> dataBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Pair* ptr = __getAddress(dataBuffer[0]);

    Pair val = *ptr;
    // CHECK: 10
    outputBuffer[0] = val.x;

    // CHECK: 20
    outputBuffer[1] = ptr->y;

    // `.` also dereferences a pointer in Slang.
    // CHECK: 10
    outputBuffer[2] = ptr.x;

    ptr.y = 99;
    // CHECK: 99
    outputBuffer[3] = dataBuffer[0].y;
}
