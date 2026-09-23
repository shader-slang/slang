//TEST:SIMPLE(filecheck=CHECK):-target spirv -g2 -O0 -emit-spirv-directly -stage compute -entry computeMain -fvk-use-entrypoint-name
//
// Regression test for shader-slang/slang#13235.
//
// A function that has a [ForceInline] callee inlined into it must still emit a
// DebugFunctionDefinition binding its OpFunction body to its DebugFunction record; otherwise a
// GPU debugger cannot map the function to its debug info and step into it. Inlining a callee makes
// the emitter insert a caller-scope-restore DebugScope inst (shader-slang/slang#13175); when a
// DebugVar (a local declared after the inlined call) follows that DebugScope, resolving its scope
// emitted the caller's DebugFunction record early and bare through the global debug-inst path,
// after which the per-body DebugFunctionDefinition was dropped on the record-cache hit in
// emitDebugFunction.
//
// The defect is not specific to entry points: it affects any function that inlines a callee and has
// a local following the restored scope. This is why the test covers both computeMain (an entry
// point) and helperCaller (an ordinary non-entry caller); plainLeaf has no inlined callee and
// always kept its definition, so it is the control. -O0 is pinned because at higher optimization
// levels helperCaller and plainLeaf would be inlined away and the scenario would no longer arise.

RWStructuredBuffer<int> outputBuffer;

[ForceInline]
int addOne(int x)
{
    return x + 1;
}

int helperCaller(int x)
{
    int first = addOne(x);
    int second = first + 5;
    return second * 2;
}

int plainLeaf(int x)
{
    int only = x * 3;
    return only;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int a = addOne(int(tid.x));
    int b = a + 7;
    int c = helperCaller(b);
    int d = plainLeaf(c);
    outputBuffer[tid.x] = d;
}

// Anchor each check on the fully-formed OpExtInst instruction so the -g2 embedded source (which
// contains these very CHECK lines as an OpString literal) cannot satisfy a match.

// The entry point that inlines addOne must keep its DebugFunctionDefinition.
// CHECK-DAG: [[CM_STR:%[0-9]+]] = OpString "computeMain"
// CHECK-DAG: [[CM_FUNC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[CM_STR]]
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunctionDefinition [[CM_FUNC]] %computeMain

// The non-entry caller that also inlines addOne must keep its DebugFunctionDefinition too.
// CHECK-DAG: [[HC_STR:%[0-9]+]] = OpString "helperCaller"
// CHECK-DAG: [[HC_FUNC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[HC_STR]]
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunctionDefinition [[HC_FUNC]] %helperCaller

// The control function with no inlined callee is unaffected and keeps its DebugFunctionDefinition.
// CHECK-DAG: [[PL_STR:%[0-9]+]] = OpString "plainLeaf"
// CHECK-DAG: [[PL_FUNC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[PL_STR]]
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunctionDefinition [[PL_FUNC]] %plainLeaf
