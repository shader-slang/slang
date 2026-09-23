//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv -g3 -O0
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


// Restore computeMain's scope before recording its local result.
// CHECK-DAG: %[[CALLER_NAME:[0-9]+]] = OpString "computeMain"
// CHECK-DAG: %[[CALLER:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction %[[CALLER_NAME]]
// CHECK-COUNT-2: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_{{[0-9]+}} %{{[0-9]+}}
// CHECK-NOT:  %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_{{[0-9]+}} %{{[0-9]+}}
// CHECK-COUNT-3: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}} %{{[0-9]+}}
// CHECK-NOT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %{{[0-9]+}} %{{[0-9]+}}
// CHECK-NOT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugNoScope
// CHECK: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %[[CALLER]]{{$}}
// CHECK-NEXT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugValue %result %{{[a-zA-Z0-9_]+}}
// CHECK-NOT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugNoScope
