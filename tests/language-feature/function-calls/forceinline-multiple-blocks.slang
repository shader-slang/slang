//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv -O0 -g3
RWStructuredBuffer<int> outputBuffer;

// This tests debuginfo for inlining a function with multiple basic blocks
// to test the more complex case of inlining with control flow.

[ForceInline]
int calculateAdjustment(int value)
{
    return value * 3 / 2;
}

[ForceInline]
int inlineMultipleBasicBlocks(int value)
{
    int result = 0;
    
    result = value * 2;
    result = calculateAdjustment(result);

    // Add another branch to create more basic blocks
    if (result > 20)
    {
        result = result + 25;
    }

    result = value * 4;
    //result = calculateAdjustment(result);

    // Add another branch to create more basic blocks
    if (result < 20)
    {
        result = result + 10;
    }
    
    return result;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int i = dispatchThreadID.x;
    
    // Call the forceinline function
    int result1 = inlineMultipleBasicBlocks(16);

    //int result2 = inlineMultipleBasicBlocks(22);

    int result3 = calculateAdjustment(2);
    
    outputBuffer[i] = result1+result3;
}

// CHECK-COUNT-3: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugFunction %{{[0-9]+}} %{{[0-9]+}} %{{[0-9]+}} %uint_{{[0-9]+}} %uint_{{[0-9]+}} %{{[0-9]+}} %{{[0-9]+}} %uint_{{[0-9]+}} %uint_{{[0-9]+}}
// CHECK-COUNT-1: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_{{[0-9]+}} %{{[0-9]+}}
// CHECK-COUNT-2: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}} %{{[0-9]+}}
// CHECK-COUNT-1: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugNoScoped
