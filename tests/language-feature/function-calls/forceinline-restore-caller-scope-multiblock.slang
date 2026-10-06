//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv -O0 -g3
RWStructuredBuffer<int> outputBuffer;

// A multi-block callee contains an inlined leaf. Returning from the leaf must
// restore the callee with its outer inline chain; returning from the callee
// must restore computeMain without an inline chain.

[ForceInline]
int leafInline(int a, int b)
{
    return a * 2 + b;
}

[ForceInline]
int multiBlockInline(int a, int b)
{
    int r = leafInline(a, b);
    if (b > 0)
        r = r + b;
    else
        r = r - b;
    return r * 2;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int i = dispatchThreadID.x;
    int result = multiBlockInline(16, 10);
    outputBuffer[i] = result;
}

// Identify the scopes for computeMain, its inlined callee, and the nested leaf.
// CHECK-DAG: %[[CMNAME:[0-9]+]] = OpString "computeMain"
// CHECK-DAG: %[[CALLER:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction %[[CMNAME]]
// CHECK-DAG: %[[MBNAME:[0-9]+]] = OpString "multiBlockInline"
// CHECK-DAG: %[[MULTI:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction %[[MBNAME]]
// CHECK-DAG: %[[LEAF_NAME:[0-9]+]] = OpString "leafInline"
// CHECK-DAG: %[[LEAF:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction %[[LEAF_NAME]]

// computeMain's own entry scope is a one-operand DebugScope.
// CHECK: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %[[CALLER]]{{$}}

// The nested leaf must restore the same scope and inline site established at
// the outer callee's entry, immediately before updating its local r.
// CHECK: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %[[MULTI]] %[[SITE:[0-9]+]]{{$}}
// CHECK: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %[[LEAF]] %{{[0-9]+}}{{$}}
// CHECK: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %[[MULTI]] %[[SITE]]{{$}}
// CHECK-NEXT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugValue %r %{{[a-zA-Z0-9_]+}}
// CHECK-NOT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %[[MULTI]]{{$}}

// After the inlined region, computeMain's own scope is restored as a one-operand DebugScope
// immediately before the caller-local `result` DebugValue.
// CHECK: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %[[CALLER]]{{$}}
// CHECK-NEXT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugValue %result

// The final restoration must leave the caller's scope active through the end of the function.
// CHECK-NOT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugNoScope
