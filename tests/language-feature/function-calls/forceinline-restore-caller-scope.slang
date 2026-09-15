//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv -O0 -g3
RWStructuredBuffer<int> outputBuffer;

// Regression test for shader-slang/slang#11616.
//
// When a [ForceInline] callee is inlined back into its caller, the SPIR-V debug info must restore
// the caller function's own DebugScope before emitting debug records for caller locals — otherwise
// the caller-local `result` DebugValue lands under no enclosing scope.

[ForceInline]
int inlineSingleBasicBlock1(int value1, int value2)
{
    return value1 * 2 + value2;
}

[ForceInline]
int inlineSingleBasicBlock2(int value1, int value2)
{
    int result1 = inlineSingleBasicBlock1(10, 20);
    return value1 * 2 + value2 + result1;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int i = dispatchThreadID.x;
    int result = inlineSingleBasicBlock2(16, 10);
    outputBuffer[i] = result;
}

// Anchor on computeMain's DebugFunction (not just "the first one-operand scope"): capture the
// "computeMain" name string, then the DebugFunction that names it.
// CHECK-DAG: %[[CMNAME:[0-9]+]] = OpString "computeMain"
// CHECK-DAG: %[[CALLER:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction %[[CMNAME]]
// Also capture inlineSingleBasicBlock2's DebugFunction; it pins the intermediate-restore check below.
// CHECK-DAG: %[[BB2NAME:[0-9]+]] = OpString "inlineSingleBasicBlock2"
// CHECK-DAG: %[[BB2:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction %[[BB2NAME]]

// computeMain's own entry scope is a one-operand DebugScope referencing that DebugFunction.
// CHECK: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %[[CALLER]]{{$}}

// inlineSingleBasicBlock2's scope appears twice under computeMain's inline context: once opening
// its own inlined body, once restoring it after the nested inlineSingleBasicBlock1 region returns.
// Both must stay two-operand — a restore nested inside an inlined region keeps that region's
// DebugInlinedAt, so only the outermost restore is one-operand.
// CHECK-COUNT-2: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %[[BB2]] %{{[0-9]+}}{{$}}
// CHECK: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugValue %result1

// The caller-local `result` DebugValue must sit under computeMain's own scope, so the restore has
// to land immediately before it — hence the CHECK-NEXT rather than a plain CHECK.
// CHECK: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %[[CALLER]]{{$}}
// CHECK-NEXT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugValue %result

// CHECK-NOT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugNoScope
