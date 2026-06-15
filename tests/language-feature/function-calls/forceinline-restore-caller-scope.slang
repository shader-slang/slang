//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv -O0 -g3
RWStructuredBuffer<int> outputBuffer;

// Regression test for shader-slang/slang#11616.
//
// When a [ForceInline] callee is inlined back into its caller, the SPIR-V debug
// info must restore the caller function's own DebugScope before emitting debug
// records for caller locals. Previously the inliner emitted a DebugNoScope here,
// clearing the active scope, so the caller-local `result` DebugValue landed under
// no enclosing scope.

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

// computeMain's own entry scope is a one-operand DebugScope referencing that DebugFunction.
// CHECK: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %[[CALLER]]{{$}}

// After the inlined callee's debug records, the caller (computeMain) function scope is restored
// as a one-operand DebugScope referencing that same DebugFunction, immediately before the
// caller-local `result` DebugValue. Previously a DebugNoScope was emitted here instead, leaving
// `result` with no enclosing scope.
// CHECK: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %[[CALLER]]{{$}}
// CHECK-NEXT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugValue %result

// The scope-clearing DebugNoScope is never emitted.
// CHECK-NOT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugNoScope
