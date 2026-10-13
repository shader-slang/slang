//TEST:SIMPLE(filecheck=CHECK): -target spirv -g3 -O0
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
[shader("compute")]
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

// Bind the call chains by their source call sites, independent of metadata order.
// Every nested call must restore the matching outer call, and every outer call
// must restore computeMain before updating its result.
// CHECK-DAG: %[[CALLER_NAME:[0-9]+]] = OpString "computeMain"
// CHECK-DAG: %[[CALLER:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction %[[CALLER_NAME]]
// CHECK-DAG: %[[OUTER_NAME:[0-9]+]] = OpString "inlineMultipleBasicBlocks"
// CHECK-DAG: %[[OUTER:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction %[[OUTER_NAME]]
// CHECK-DAG: %[[INNER_NAME:[0-9]+]] = OpString "calculateAdjustment"
// CHECK-DAG: %[[INNER:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction %[[INNER_NAME]]
// CHECK-DAG: %[[A:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_62 %[[CALLER]]{{$}}
// CHECK-DAG: %[[B:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_64 %[[CALLER]]{{$}}
// CHECK-DAG: %[[C:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_66 %[[CALLER]]{{$}}
// CHECK-DAG: %[[A1:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_24 %[[OUTER]] %[[A]]{{$}}
// CHECK-DAG: %[[A2:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_37 %[[OUTER]] %[[A]]{{$}}
// CHECK-DAG: %[[A3:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_46 %[[OUTER]] %[[A]]{{$}}
// CHECK-DAG: %[[B1:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_24 %[[OUTER]] %[[B]]{{$}}
// CHECK-DAG: %[[B2:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_37 %[[OUTER]] %[[B]]{{$}}
// CHECK-DAG: %[[B3:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugInlinedAt %uint_46 %[[OUTER]] %[[B]]{{$}}
// CHECK-DAG: %[[BRANCH:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugLexicalBlock %{{[0-9]+}} %uint_29 %uint_9 %[[OUTER]]
// CHECK: DebugScope %[[INNER]] %[[A1]]{{$}}
// CHECK: DebugScope %[[OUTER]] %[[A]]{{$}}
// CHECK-NEXT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugValue %result %int_48
// CHECK: DebugScope %[[BRANCH]] %[[A]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: DebugValue %result %int_73
// CHECK: DebugScope %[[INNER]] %[[A2]]{{$}}
// CHECK: DebugScope %[[OUTER]] %[[A]]{{$}}
// CHECK-NEXT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugValue %result %int_96
// CHECK: DebugScope %[[INNER]] %[[A3]]{{$}}
// CHECK: DebugScope %[[OUTER]] %[[A]]{{$}}
// CHECK-NEXT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugValue %result %int_240
// CHECK: DebugScope %[[CALLER]]{{$}}
// CHECK-NEXT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugValue %result1 %int_340
// CHECK: DebugScope %[[INNER]] %[[B1]]{{$}}
// CHECK: DebugScope %[[OUTER]] %[[B]]{{$}}
// CHECK-NEXT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugValue %result_0 %int_66
// CHECK: DebugScope %[[BRANCH]] %[[B]]{{$}}
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: DebugValue %result_0 %int_91
// CHECK: DebugScope %[[INNER]] %[[B2]]{{$}}
// CHECK: DebugScope %[[OUTER]] %[[B]]{{$}}
// CHECK-NEXT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugValue %result_0 %int_132
// CHECK: DebugScope %[[INNER]] %[[B3]]{{$}}
// CHECK: DebugScope %[[OUTER]] %[[B]]{{$}}
// CHECK-NEXT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugValue %result_0 %int_330
// CHECK: DebugScope %[[CALLER]]{{$}}
// CHECK-NEXT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugValue %result2 %int_330
// CHECK: DebugScope %[[INNER]] %[[C]]{{$}}
// CHECK: DebugScope %[[CALLER]]{{$}}
// CHECK-NEXT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugValue %result3 %int_3
// CHECK-NOT: Debug{{(No)?Scope}}
// CHECK: OpReturn
