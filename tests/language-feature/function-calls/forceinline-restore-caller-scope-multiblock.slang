//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv -O0 -g3
RWStructuredBuffer<int> outputBuffer;

// Regression test for shader-slang/slang#11616, exercising the *multi-basic-block* inlining path.
//
// The other restore-caller-scope tests use single-basic-block [ForceInline] callees, which take
// the single-block clone path in the inliner. This test uses a callee with control flow (if/else),
// which has more than one IR block and therefore takes the multi-block clone path
// (inlineMultipleBlockFuncBody). It also nests a leaf [ForceInline] call inside that multi-block
// callee, so the multi-block clone loop's one-operand restore-scope upgrade (debugScopeToUpgrade)
// runs. Both paths must still restore the caller's own one-operand DebugScope after the inlined
// region and never emit a scope-clearing DebugNoScope.

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

// Anchor on computeMain's DebugFunction, and on the multi-block callee's DebugFunction (used below
// to pin the multi-block restore upgrade).
// CHECK-DAG: %[[CMNAME:[0-9]+]] = OpString "computeMain"
// CHECK-DAG: %[[CALLER:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction %[[CMNAME]]
// CHECK-DAG: %[[MBNAME:[0-9]+]] = OpString "multiBlockInline"
// CHECK-DAG: %[[MULTI:[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction %[[MBNAME]]

// computeMain's own entry scope is a one-operand DebugScope.
// CHECK: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %[[CALLER]]{{$}}

// The multi-block callee is inlined with inline depth: its scope appears only as a *two-operand*
// DebugScope (scope + DebugInlinedAt) under computeMain's inline context — both the per-block scopes
// and the intermediate restore left after the nested leafInline region returns. The multi-block
// clone loop's debugScopeToUpgrade step re-attaches the outer call's DebugInlinedAt to that restore;
// if it were dropped, the restore would collapse to a one-operand DebugScope %[[MULTI]], which the
// CHECK-NOT below rejects across the whole inlined region (up to computeMain's own restore).
// CHECK: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %[[MULTI]] %{{[0-9]+}}{{$}}
// CHECK-NOT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %[[MULTI]]{{$}}

// After the inlined region, computeMain's own scope is restored as a one-operand DebugScope
// immediately before the caller-local `result` DebugValue.
// CHECK: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugScope %[[CALLER]]{{$}}
// CHECK-NEXT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugValue %result

// The scope-clearing DebugNoScope is never emitted.
// CHECK-NOT: %{{[0-9]+}} = OpExtInst %void %{{[0-9]+}} DebugNoScope
