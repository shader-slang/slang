//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv -O0 -g3
RWStructuredBuffer<int> outputBuffer;

// This tests debuginfo for inlining a function with a single basic block.
[ForceInline]
int inlineSingleBasicBlock(int value1, int value2)
{
    // Simple operation that should be inlined
    return value1 * 2 + value2;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int i = dispatchThreadID.x;
    
    // Call the forceinline function
    int result = inlineSingleBasicBlock(16, 10);
    
    outputBuffer[i] = result;
}


// TODO1: This is now in a different section. Modify test accordingly.
// TODO_CHECK: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_18 %{{[0-9]+}}

// TODO2: Spirv validation error for DebugInlinedVariable. Check this.
// The emission is now moved to the appropriate section, but seeing this error.
//error: line 64: ID '60[%value1]' has not been defined
//  %59 = OpExtInst %void %2 DebugInlinedVariable %value1 %57

// CHECK: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}}
// CHECK: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugNoScope
