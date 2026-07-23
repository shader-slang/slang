// struct-value-semantics.slang

// Test that assignment copies the struct (value semantics) —
// modifying the copy does not affect the original.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct Data
{
    int x;
    int y;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // Create original
    Data original;
    original.x = 100;
    original.y = 200;

    // Copy via assignment (value semantics)
    Data copy = original;

    // Modify the copy
    copy.x = 999;
    copy.y = 888;

    // Original should be unaffected
    // CHECK: 100
    outputBuffer[0] = original.x;

    // CHECK: 200
    outputBuffer[1] = original.y;

    // Copy has the new values
    // CHECK: 999
    outputBuffer[2] = copy.x;

    // CHECK: 888
    outputBuffer[3] = copy.y;
}
