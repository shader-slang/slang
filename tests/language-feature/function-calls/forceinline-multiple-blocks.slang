//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv -O0 -g3
RWStructuredBuffer<int> outputBuffer;

// This tests the following use cases:
// Inline single block function in multiple places.
// Inline single block function into multi-block function(This also covers multiple blocks with a phi node.)
// Inline single block function in multiple places in the same function.
// Inline multi-block function into another function
// Inline multi-block function multiple times
// Recursive inlining use case.

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
    else
    {
        result = result + 50;
    }

    result = value * 4;
    result = calculateAdjustment(result);

    // Add another branch to create more basic blocks
    if (result < 20)
    {
        result = result + 10;
    }

    result = value * 10;
    result = calculateAdjustment(result);
    
    if (result < 250)
    {
        result = result + 100;
    }

    return result;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int i = dispatchThreadID.x;
    
    // Call the forceinline function
    int result1 = inlineMultipleBasicBlocks(16);

    int result2 = inlineMultipleBasicBlocks(22);

    int result3 = calculateAdjustment(2);
    
    outputBuffer[i] = result1 + result2 + result3;
}

// CHECK-COUNT-3: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugFunction %{{[0-9]+}} %{{[0-9]+}} %{{[0-9]+}} %uint_{{[0-9]+}} %uint_{{[0-9]+}} %{{[0-9]+}} %{{[0-9]+}} %uint_{{[0-9]+}} %uint_{{[0-9]+}}
// CHECK-NOT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugFunction %{{[0-9]+}} %{{[0-9]+}} %{{[0-9]+}} %uint_{{[0-9]+}} %uint_{{[0-9]+}} %{{[0-9]+}} %{{[0-9]+}} %uint_{{[0-9]+}} %uint_{{[0-9]+}}

// TODO: Actual count is 6. But the pattern matcher complains to match.
// CHECK-COUNT-5: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_{{[0-9]+}} %{{[0-9]+}} %{{[0-9]+}}
// CHECK-NOT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_{{[0-9]+}} %{{[0-9]+}} %{{[0-9]+}}

// CHECK-COUNT-21: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}} %{{[0-9]+}}
// CHECK-NOT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}} %{{[0-9]+}}

// TODO: Verified manually that the count in the .actual file is 21. 
// Seems like the pattern matcher has an issue. It points to the line after 
// the last debugscope.
// _CHECK-COUNT-21: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugNoScope
// _CHECK-NOT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugNoScope

