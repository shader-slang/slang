//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUFFER):-shaderobj -vk
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUFFER):-shaderobj -mtl
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUFFER):-cpu -compute -entry computeMain

//TEST_INPUT:ubuffer(data=[9 9 9 9], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

void test<let TSize : uint>()
{
    // Test array initialization with generic size
    int array[TSize] = {};

    // BUFFER: 0
    outputBuffer[0] = array[0];
    // BUFFER-NEXT: 0
    outputBuffer[1] = array[1];
    // BUFFER-NEXT: 0
    outputBuffer[2] = array[2];
    // BUFFER-NEXT: 3
    outputBuffer[3] = array.getCount();
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    test<3>();
} 
