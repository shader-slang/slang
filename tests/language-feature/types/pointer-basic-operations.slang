// pointer-basic-operations.slang

// Test basic pointer declaration, dereference with *, ->, and . (all three forms).

// DISABLED: __getAddress on a structured-buffer element (buf[i]) is rejected as a
// function-local address on every target (spirv/cuda/cpp), not just GPU. Tracked by
// shader-slang/slang#12581. Re-enable with a -cpu directive once that is fixed.
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct Pair
{
    int x;
    int y;
}

//TEST_INPUT:ubuffer(data=[10 20 0 0], stride=4):name=dataBuffer
RWStructuredBuffer<Pair> dataBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Get a pointer to the first element
    Pair* ptr = __getAddress(dataBuffer[0]);

    // Dereference with * operator
    Pair val = *ptr;
    // CHECK: 10
    outputBuffer[0] = val.x;

    // Dereference with -> operator
    // CHECK: 20
    outputBuffer[1] = ptr->y;

    // Dereference with . operator (dot also dereferences pointers in Slang)
    // CHECK: 10
    outputBuffer[2] = ptr.x;

    // Write through pointer using . dereference
    ptr.y = 99;
    // CHECK: 99
    outputBuffer[3] = dataBuffer[0].y;
}
