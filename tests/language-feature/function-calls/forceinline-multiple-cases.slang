//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv -O0 -g3
RWStructuredBuffer<int> outputBuffer;

// Function with a single basic block
[ForceInline]
int basicBlockFunc(int value1, int value2)
{
    // Simple operation that should be inlined
    return value1 * 2 + value2;
}

// Function with multiple basic blocks
[ForceInline]
int multipleBlockFunc(int value1, int value2)
{
    int result = value1 * 2;
    
    // Add a condition to create multiple basic blocks
    if (value1 > value2)
    {
        result += value2 * 3;
    }
    else
    {
        result -= value2;
    }
    
    return result;
}

// Test case a: Multiple calls to basic block function
int testMultipleBasicBlockCalls()
{
    int result = basicBlockFunc(10, 5);
    result += basicBlockFunc(20, 15);
    result += basicBlockFunc(30, 25);
    return result;
}

// Test case b: Multiple calls to multiple block function
int testMultipleBlockCalls()
{
    int result = multipleBlockFunc(10, 5);
    result += multipleBlockFunc(20, 25);
    result += multipleBlockFunc(30, 25);
    return result;
}

// Test case c: One call to basic block, other instructions, another call to basic block
int testBasicBlockWithInstructions()
{
    int result = basicBlockFunc(10, 5);
    
    // Some other instructions
    result *= 2;
    result += 10;
    
    result += basicBlockFunc(20, 15);
    return result;
}

// Test case d: One call to multiple block func, other instructions, another call to multiple block func
int testMultipleBlockWithInstructions()
{
    int result = multipleBlockFunc(10, 5);
    
    // Some other instructions
    result *= 2;
    result += 10;
    
    result += multipleBlockFunc(20, 25);
    return result;
}

// Test case e: One call to basic block, other instructions, call to multiple block
int testBasicToMultipleBlock()
{
    int result = basicBlockFunc(10, 5);
    
    // Some other instructions
    result *= 2;
    result += 10;
    
    result += multipleBlockFunc(20, 25);
    return result;
}

// Additional test case: Mixed calls with condition
int testMixedCallsWithCondition()
{
    int result = basicBlockFunc(15, 7);
    if (result > 30)
    {
        result += multipleBlockFunc(8, 3);
    }
    else
    {
        result += multipleBlockFunc(12, 9);
    }
    return result;
}

[numthreads(8, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Call all test functions directly and store results in different indices
    
    // Test case a: Multiple calls to basic block function
    outputBuffer[0] = testMultipleBasicBlockCalls();
    
    // Test case b: Multiple calls to multiple block function
    outputBuffer[1] = testMultipleBlockCalls();
    
    // Test case c: One call to basic block, other instructions, another call to basic block
    outputBuffer[2] = testBasicBlockWithInstructions();
    
    // Test case d: One call to multiple block func, other instructions, another call to multiple block func
    outputBuffer[3] = testMultipleBlockWithInstructions();
    
    // Test case e: One call to basic block, other instructions, call to multiple block
    outputBuffer[4] = testBasicToMultipleBlock();
    
    // Additional test case: Mixed calls with condition
    outputBuffer[5] = testMixedCallsWithCondition();
    
    // Set any remaining indices to 0
    if (dispatchThreadID.x >= 6)
    {
        outputBuffer[dispatchThreadID.x] = 0;
    }
}

// CHECK-COUNT-14: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_{{[0-9]+}} %{{[0-9]+}}
// CHECK-COUNT-14: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugNoScope