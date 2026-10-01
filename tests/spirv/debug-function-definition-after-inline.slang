//TEST:SIMPLE(filecheck=CHECK):-target spirv -g2 -O0 -emit-spirv-directly -stage compute -entry computeMain -fvk-use-entrypoint-name
//
// A function that inlines a [ForceInline] callee and declares a local AFTER the inlined call
// must still emit a DebugFunctionDefinition binding its OpFunction body to its DebugFunction
// record, so a debugger can associate the OpFunction with its debug info. Inlining makes the
// inliner add a caller-scope-restore DebugScope referencing the caller's own DebugFunction; the
// local's DebugVar then follows that DebugScope, and handling that ordering is the emitter's job.
// A local assigned directly at the call, with nothing after it, does not exercise this.
// See shader-slang/slang#13235.
//
// The defect is not specific to entry points, so this covers both computeMain (an entry point) and
// helperCaller (a non-entry caller); plainLeaf has no inlined callee and is the unaffected control.
// -O0 is pinned so helperCaller and plainLeaf are not inlined away, and -fvk-use-entrypoint-name
// keeps the entry point's OpFunction named %computeMain so the computeMain CHECK below matches by
// name instead of silently failing on the default %main.

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

// CHECK-DAG: [[CM_STR:%[0-9]+]] = OpString "computeMain"
// CHECK-DAG: [[CM_FUNC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[CM_STR]]
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunctionDefinition [[CM_FUNC]] %computeMain

// CHECK-DAG: [[HC_STR:%[0-9]+]] = OpString "helperCaller"
// CHECK-DAG: [[HC_FUNC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[HC_STR]]
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunctionDefinition [[HC_FUNC]] %helperCaller

// A pre-inline DebugVar (here helperCaller's parameter x) in a bare-cached function must scope to
// that function's DebugFunction, not the module DebugCompilationUnit; this guards the
// cache-hit-path registerDebugInst so the OpDebugLocalVariable Parent resolves to helperCaller.
// CHECK-DAG: [[X_STR:%[0-9]+]] = OpString "x"
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugLocalVariable [[X_STR]] {{.*}} [[HC_FUNC]] %uint_0

// CHECK-DAG: [[PL_STR:%[0-9]+]] = OpString "plainLeaf"
// CHECK-DAG: [[PL_FUNC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[PL_STR]]
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunctionDefinition [[PL_FUNC]] %plainLeaf
