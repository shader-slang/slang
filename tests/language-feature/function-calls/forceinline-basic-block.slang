//TEST:SIMPLE(filecheck=CHECK): -target spirv -g3 -O0
RWStructuredBuffer<int> outputBuffer;

[ForceInline]
int inlineSingleBasicBlock1(int value1, int value2)
{
    // Simple operation that should be inlined
    return value1 * 2 + value2;
}

[ForceInline]
int inlineSingleBasicBlock2(int value1, int value2)
{
    int result1 = inlineSingleBasicBlock1(10, 20);
    // Simple operation that should be inlined
    return value1 * 2 + value2 + result1;
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int i = dispatchThreadID.x;

    // Call the forceinline function
    int result = inlineSingleBasicBlock2(16, 10);

    outputBuffer[i] = result;
}


// Track both call sites and verify that returning from the inner call restores
// its inlined caller before returning to computeMain. Repeated scope markers
// are harmless; each result must immediately follow the correct restoration.
// CHECK-DAG: %[[CALLER_NAME:[0-9]+]] = OpString "computeMain"
// CHECK-DAG: %[[OUTER_NAME:[0-9]+]] = OpString "inlineSingleBasicBlock2"
// CHECK-DAG: %[[INNER_NAME:[0-9]+]] = OpString "inlineSingleBasicBlock1"
// CHECK-DAG: %[[CALLER:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction %[[CALLER_NAME]]
// CHECK-DAG: %[[OUTER:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction %[[OUTER_NAME]]
// CHECK-DAG: %[[INNER:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction %[[INNER_NAME]]
// CHECK-DAG: %[[OUTER_CALL:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_{{[0-9]+}} %[[CALLER]]{{$}}
// CHECK-DAG: %[[INNER_CALL:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_{{[0-9]+}} %[[OUTER]] %[[OUTER_CALL]]{{$}}
// CHECK: DebugScope %[[OUTER]] %[[OUTER_CALL]]{{$}}
// CHECK: DebugScope %[[INNER]] %[[INNER_CALL]]{{$}}
// CHECK: DebugScope %[[OUTER]] %[[OUTER_CALL]]{{$}}
// CHECK-NEXT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugValue %result1 %int_40
// CHECK: DebugScope %[[CALLER]]{{$}}
// CHECK-NEXT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugValue %result %int_82
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: OpReturn
