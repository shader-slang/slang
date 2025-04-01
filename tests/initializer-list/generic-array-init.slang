//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUFFER):-shaderobj -vk
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUFFER):-shaderobj -mtl
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUFFER):-cpu -compute -entry computeMain

// Test array initialization with generic size arguments
struct ArrayTest<let TSize : uint> {
    int array[TSize];
}

struct ArrayTestDefault<let TSize : uint> {
    int array[TSize] = {};
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 1 2 3 4], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

void test()
{
    // Test array initialization with generic size
    ArrayTest<3> arr1 = { {1, 2, 3} };
    // BUFFER: 1
    outputBuffer[0] = arr1.array[0];
    // BUFFER-NEXT: 2
    outputBuffer[1] = arr1.array[1];
    // BUFFER-NEXT: 3
    outputBuffer[2] = arr1.array[2];

    // Test nested array initialization
    ArrayTest<2> arr2 = { {4, 5} };
    // BUFFER-NEXT: 4
    outputBuffer[3] = arr2.array[0];
    // BUFFER-NEXT: 5
    outputBuffer[4] = arr2.array[1];

    // Test empty array initialization
    ArrayTest<2> arr3 = {};
    // BUFFER-NEXT: 0
    outputBuffer[5] = arr3.array[0];
    // BUFFER-NEXT: 0
    outputBuffer[6] = arr3.array[1];

    ArrayTestDefault<2> arr4;
    // BUFFER-NEXT: 0
    outputBuffer[7] = arr4.array[0];
    // BUFFER-NEXT: 0
    outputBuffer[8] = arr4.array[1];
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    test();
} 
