//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv -g3
RWStructuredBuffer<int> outputBuffer;

// Test where outer function is inlined before the inner function.

[ForceInline]
int inlineSingleBasicBlock1(int value1, int value2)
{
    // Simple operation that should be inlined
    return value1 * 2 + value2;
}

[__unsafeForceInlineEarly]
int inlineSingleBasicBlock2(int value1, int value2)
{
    int result1 = inlineSingleBasicBlock1(10, 20);
    // Simple operation that should be inlined
    return value1 * 2 + value2 + result1;
}


[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int i = dispatchThreadID.x;
    
    // Call the forceinline function
    int result = inlineSingleBasicBlock2(16, 10);
    
    outputBuffer[i] = result;
}


// CHECK-COUNT-2: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_{{[0-9]+}} %{{[0-9]+}}
// CHECK-NOT:  %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_{{[0-9]+}} %{{[0-9]+}}
// CHECK-COUNT-3: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}} %{{[0-9]+}}
// CHECK-NOT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}} %{{[0-9]+}}
// After the inlined region the caller's own function scope is restored as a one-operand
// DebugScope (instead of the previous scope-clearing DebugNoScope). See shader-slang/slang#11616.
// CHECK: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}}{{$}}
// Exactly one top-level one-operand restore is expected; a spurious second would be a bug.
// CHECK-NOT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}}{{$}}
// CHECK-NOT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugNoScope