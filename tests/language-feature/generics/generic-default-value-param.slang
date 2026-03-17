// Test generic value parameters with default values.
// When no explicit value argument is provided, the default value should be used.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type

struct FixedArray<let N : uint = 4>
{
    int elems[N];

    int sum()
    {
        int result = 0;
        for (uint i = 0; i < N; i++)
            result = result + elems[i];
        return result;
    }
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Default N = 4: array has 4 elements
    FixedArray<> arr4;
    arr4.elems[0] = 1;
    arr4.elems[1] = 2;
    arr4.elems[2] = 3;
    arr4.elems[3] = 4;
    outputBuffer[0] = arr4.sum();
    // CHECK: 10

    // Explicit N = 2: array has 2 elements
    FixedArray<2> arr2;
    arr2.elems[0] = 10;
    arr2.elems[1] = 20;
    outputBuffer[1] = arr2.sum();
    // CHECK: 30
}
