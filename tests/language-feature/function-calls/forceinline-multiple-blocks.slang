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
    
    // Create multiple basic blocks with conditional statements
    if (value > 10)
    {
        result = value * 2;
        result = calculateAdjustment(result);
    }
    else
    {
        result = value + 5;
    }
    
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
    int result = inlineMultipleBasicBlocks(16);
    
    outputBuffer[i] = result;
}

// CHECK-COUNT-3: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugFunction %{{[0-9]+}} %{{[0-9]+}} %{{[0-9]+}} %uint_{{[0-9]+}} %uint_{{[0-9]+}} %{{[0-9]+}} %{{[0-9]+}} %uint_{{[0-9]+}} %uint_{{[0-9]+}}
// CHECK-COUNT-1: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_{{[0-9]+}} %{{[0-9]+}}
// CHECK-COUNT-7: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}} %{{[0-9]+}}
// CHECK-COUNT-1: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugNoScope
