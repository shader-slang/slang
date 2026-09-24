//TEST:SIMPLE(filecheck=CHECK):-target spirv -g2 -O0 -emit-spirv-directly -stage compute -entry computeMain

// Regression guard for shader-slang/slang#13238: reverse-mode autodiff must give each generated
// derivative (s_apply_ / s_bwdProp_ / ...) its OWN DebugFunction record, named after the derivative
// and bound to its own body, rather than sharing the original function's record. A shared record
// reports a derivative body under the original's name and, because a DebugFunctionDefinition binds
// one record to one body, leaves the other sharing bodies undescribed. The bug is invisible to
// spirv-val (NonSemantic debug info is not validated), so an explicit FileCheck guard is required.
//
// The loop forces the reverse-mode primal/propagate split that produces the s_apply_ / s_bwdProp_
// bodies; the [ForceInline] differentiable callee adds an inlined-callee body to the set. Record and
// name ids are captured (not source text) so the -g2 embedded source cannot satisfy the match.

RWStructuredBuffer<float> outputBuffer;

[ForceInline]
[Differentiable]
float addOneD(float x)
{
    return x + 1.0;
}

[Differentiable]
float f(float x)
{
    float acc = addOneD(x);
    float extra = acc * 0.5;
    [MaxIters(4)]
    for (int i = 0; i < 4; i++)
    {
        acc = acc * acc + extra;
    }
    return acc;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    DifferentialPair<float> dp = diffPair(2.0, 0.0);
    bwd_diff(f)(dp, 1.0);
    outputBuffer[tid.x] = dp.d;
}

// s_apply_f and s_bwdProp_f each own a distinct DebugFunction record, named after themselves rather
// than sharing the original "f" record, and each record is bound to its own body by a
// DebugFunctionDefinition.
// CHECK-DAG: [[APPLY_STR:%[0-9]+]] = OpString "s_apply_f"
// CHECK-DAG: [[BWD_STR:%[0-9]+]] = OpString "s_bwdProp_f"
// CHECK-DAG: [[APPLY_REC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[APPLY_STR]]
// CHECK-DAG: [[BWD_REC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[BWD_STR]]
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunctionDefinition [[APPLY_REC]]
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunctionDefinition [[BWD_REC]]
// s_bwdProp_f's record binds to exactly one body -- a duplicate definition would be a regression.
// CHECK-NOT: OpExtInst %void %{{[0-9]+}} DebugFunctionDefinition [[BWD_REC]]
