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


// The below checks for the three top-level inlines from computeMain, and then for the first two of
// those, that they each have the inner function (calculateAdjustment) inlined three times, and then
// that there are no more inlines in the file.

// CHECK: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_{{[0-9]+}} %{{[0-9]+}}
// CHECK-COUNT-3: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_{{[0-9]+}} %{{[0-9]+}} %{{[0-9]+}}
// CHECK: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_{{[0-9]+}} %{{[0-9]+}}
// CHECK-COUNT-3: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_{{[0-9]+}} %{{[0-9]+}} %{{[0-9]+}}
// CHECK: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_{{[0-9]+}} %{{[0-9]+}}
//
// CHECK-NOT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugInlinedAt

// CHECK-COUNT-27: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}} %{{[0-9]+}}
// CHECK-NOT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}} %{{[0-9]+}}
