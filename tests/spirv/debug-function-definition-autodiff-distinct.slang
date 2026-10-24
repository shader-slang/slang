//TEST:SIMPLE(filecheck=CHECK):-target spirv -g2 -O0 -emit-spirv-directly -stage compute -entry computeMain -validate-ir
//TEST:SIMPLE(filecheck=FREC):-target spirv -g2 -O0 -emit-spirv-directly -stage compute -entry computeMain -validate-ir
//TEST:SIMPLE(filecheck=SCOPE):-target spirv -g2 -O0 -emit-spirv-directly -stage compute -entry computeMain -validate-ir
//TEST:SIMPLE(filecheck=NOFWD):-target spirv -g2 -O0 -emit-spirv-directly -stage compute -entry computeMain -validate-ir

// Regression guard for shader-slang/slang#13238: reverse-mode autodiff must give each generated
// derivative (s_apply_ / s_bwdProp_ / ...) its OWN DebugFunction record, named after the derivative
// and bound to its own body, rather than sharing the original function's record. A shared record
// reports a derivative body under the original's name and, because a DebugFunctionDefinition binds
// one record to one body, leaves the other sharing bodies undescribed. The bug is invisible to
// spirv-val (NonSemantic debug info is not validated), so an explicit FileCheck guard is required.
//
// The loop forces the reverse-mode primal/propagate split that produces the s_apply_ / s_bwdProp_
// bodies; the [ForceInline] differentiable callee adds an inlined-callee body to the set. Calling f
// directly as well keeps f's own body and record alive next to its derivatives. Record and name ids
// are captured (not source text) so the -g2 embedded source cannot satisfy the match.
//
// A derivative's body is transcribed from a clone of f, so its DebugScope / DebugLocalVariable /
// references start out naming f's scope tree. They must be re-rooted onto the derivative's own record;
// otherwise the body claims to be in "f" while its definition binds the derivative's record, and a
// stale "f" record with no definition survives. The FREC checks require exactly one "f" record, bound
// to f's body; the SCOPE checks require every non-inlined DebugScope in each derivative to name the
// derivative's own record or one of its lexical blocks.
//
// Companion to tests/autodiff/autodiff-debug-function-per-derivative.slang, which covers concrete
// and generic functions in both directions; this file uniquely exercises the reverse-mode loop /
// [ForceInline] scenario.

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
    outputBuffer[tid.x] = dp.d + f(3.0);
}

// s_apply_f and s_bwdProp_f each own a distinct DebugFunction record, named after themselves rather
// than sharing the original "f" record, and each record is bound to its own body by a
// DebugFunctionDefinition.
// CHECK-DAG: [[APPLY_STR:%[0-9]+]] = OpString "s_apply_f"
// CHECK-DAG: [[BWD_STR:%[0-9]+]] = OpString "s_bwdProp_f"
// CHECK-DAG: [[APPLY_REC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[APPLY_STR]] %
// CHECK-DAG: [[BWD_REC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[BWD_STR]] %
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunctionDefinition [[APPLY_REC]] %
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugFunctionDefinition [[BWD_REC]] %

// FREC: [[F_STR:%[0-9]+]] = OpString "f"
// FREC: [[F_REC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[F_STR]] %
// FREC-NOT: OpExtInst %void %{{[0-9]+}} DebugFunction [[F_STR]] %
// FREC: OpExtInst %void %{{[0-9]+}} DebugFunctionDefinition [[F_REC]] %f{{$}}
// FREC-NOT: OpExtInst %void %{{[0-9]+}} DebugFunctionDefinition [[F_REC]] %
// The reverse-mode bodies are split out of an intermediate forward derivative. This test calls no
// fwd_diff, so a record named after it survives only if their scopes were not re-rooted.
// NOFWD-NOT: = OpString "s_fwd_f"

// Every DebugFunction record and lexical block in the module is captured. Within each derivative's
// OpFunction, a non-inlined DebugScope (one with no inlinedAt operand) naming any captured record or
// block other than the derivative's own fails the check, and at least one DebugScope must name the
// derivative's own record (or, for s_apply_f, its re-rooted outer block).
// SCOPE-DAG: [[S_CM_STR:%[0-9]+]] = OpString "computeMain"
// SCOPE-DAG: [[S_AF_STR:%[0-9]+]] = OpString "s_apply_f"
// SCOPE-DAG: [[S_AA_STR:%[0-9]+]] = OpString "s_apply_addOneD"
// SCOPE-DAG: [[S_BF_STR:%[0-9]+]] = OpString "s_bwdProp_f"
// SCOPE-DAG: [[S_BA_STR:%[0-9]+]] = OpString "s_bwdProp_addOneD"
// SCOPE-DAG: [[S_F_STR:%[0-9]+]] = OpString "f"
// SCOPE-DAG: [[S_A_STR:%[0-9]+]] = OpString "addOneD"
// SCOPE-DAG: [[S_CM:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[S_CM_STR]] %
// SCOPE-DAG: [[S_AF:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[S_AF_STR]] %
// SCOPE-DAG: [[S_AA:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[S_AA_STR]] %
// SCOPE-DAG: [[S_BF:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[S_BF_STR]] %
// SCOPE-DAG: [[S_BA:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[S_BA_STR]] %
// SCOPE-DAG: [[S_F:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[S_F_STR]] %
// SCOPE-DAG: [[S_A:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[S_A_STR]] %
// SCOPE-DAG: [[S_AF_B1:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugLexicalBlock %{{[0-9]+}} %{{[a-z_0-9]+}} %{{[a-z_0-9]+}} [[S_AF]]{{$}}
// SCOPE-DAG: [[S_AF_B2:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugLexicalBlock %{{[0-9]+}} %{{[a-z_0-9]+}} %{{[a-z_0-9]+}} [[S_AF_B1]]{{$}}
// SCOPE-DAG: [[S_F_B1:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugLexicalBlock %{{[0-9]+}} %{{[a-z_0-9]+}} %{{[a-z_0-9]+}} [[S_F]]{{$}}
// SCOPE-DAG: [[S_F_B2:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugLexicalBlock %{{[0-9]+}} %{{[a-z_0-9]+}} %{{[a-z_0-9]+}} [[S_F_B1]]{{$}}
// SCOPE-LABEL: %s_apply_f = OpFunction %
// SCOPE-NOT: DebugScope [[S_CM]]{{$}}
// SCOPE-NOT: DebugScope [[S_AA]]{{$}}
// SCOPE-NOT: DebugScope [[S_BF]]{{$}}
// SCOPE-NOT: DebugScope [[S_BA]]{{$}}
// SCOPE-NOT: DebugScope [[S_F]]{{$}}
// SCOPE-NOT: DebugScope [[S_F_B1]]{{$}}
// SCOPE-NOT: DebugScope [[S_F_B2]]{{$}}
// SCOPE-NOT: DebugScope [[S_A]]{{$}}
// SCOPE: DebugScope [[S_AF_B1]]{{$}}
// SCOPE-NOT: DebugScope [[S_CM]]{{$}}
// SCOPE-NOT: DebugScope [[S_AA]]{{$}}
// SCOPE-NOT: DebugScope [[S_BF]]{{$}}
// SCOPE-NOT: DebugScope [[S_BA]]{{$}}
// SCOPE-NOT: DebugScope [[S_F]]{{$}}
// SCOPE-NOT: DebugScope [[S_F_B1]]{{$}}
// SCOPE-NOT: DebugScope [[S_F_B2]]{{$}}
// SCOPE-NOT: DebugScope [[S_A]]{{$}}
// SCOPE-LABEL: %s_apply_addOneD = OpFunction %
// SCOPE-NOT: DebugScope [[S_CM]]{{$}}
// SCOPE-NOT: DebugScope [[S_AF]]{{$}}
// SCOPE-NOT: DebugScope [[S_AF_B1]]{{$}}
// SCOPE-NOT: DebugScope [[S_AF_B2]]{{$}}
// SCOPE-NOT: DebugScope [[S_BF]]{{$}}
// SCOPE-NOT: DebugScope [[S_BA]]{{$}}
// SCOPE-NOT: DebugScope [[S_F]]{{$}}
// SCOPE-NOT: DebugScope [[S_F_B1]]{{$}}
// SCOPE-NOT: DebugScope [[S_F_B2]]{{$}}
// SCOPE-NOT: DebugScope [[S_A]]{{$}}
// SCOPE: DebugScope [[S_AA]]{{$}}
// SCOPE-NOT: DebugScope [[S_CM]]{{$}}
// SCOPE-NOT: DebugScope [[S_AF]]{{$}}
// SCOPE-NOT: DebugScope [[S_AF_B1]]{{$}}
// SCOPE-NOT: DebugScope [[S_AF_B2]]{{$}}
// SCOPE-NOT: DebugScope [[S_BF]]{{$}}
// SCOPE-NOT: DebugScope [[S_BA]]{{$}}
// SCOPE-NOT: DebugScope [[S_F]]{{$}}
// SCOPE-NOT: DebugScope [[S_F_B1]]{{$}}
// SCOPE-NOT: DebugScope [[S_F_B2]]{{$}}
// SCOPE-NOT: DebugScope [[S_A]]{{$}}
// SCOPE-LABEL: %s_bwdProp_f = OpFunction %
// SCOPE-NOT: DebugScope [[S_CM]]{{$}}
// SCOPE-NOT: DebugScope [[S_AF]]{{$}}
// SCOPE-NOT: DebugScope [[S_AF_B1]]{{$}}
// SCOPE-NOT: DebugScope [[S_AF_B2]]{{$}}
// SCOPE-NOT: DebugScope [[S_AA]]{{$}}
// SCOPE-NOT: DebugScope [[S_BA]]{{$}}
// SCOPE-NOT: DebugScope [[S_F]]{{$}}
// SCOPE-NOT: DebugScope [[S_F_B1]]{{$}}
// SCOPE-NOT: DebugScope [[S_F_B2]]{{$}}
// SCOPE-NOT: DebugScope [[S_A]]{{$}}
// SCOPE: DebugScope [[S_BF]]{{$}}
// SCOPE-NOT: DebugScope [[S_CM]]{{$}}
// SCOPE-NOT: DebugScope [[S_AF]]{{$}}
// SCOPE-NOT: DebugScope [[S_AF_B1]]{{$}}
// SCOPE-NOT: DebugScope [[S_AF_B2]]{{$}}
// SCOPE-NOT: DebugScope [[S_AA]]{{$}}
// SCOPE-NOT: DebugScope [[S_BA]]{{$}}
// SCOPE-NOT: DebugScope [[S_F]]{{$}}
// SCOPE-NOT: DebugScope [[S_F_B1]]{{$}}
// SCOPE-NOT: DebugScope [[S_F_B2]]{{$}}
// SCOPE-NOT: DebugScope [[S_A]]{{$}}
// SCOPE-LABEL: %s_bwdProp_addOneD = OpFunction %
// SCOPE-NOT: DebugScope [[S_CM]]{{$}}
// SCOPE-NOT: DebugScope [[S_AF]]{{$}}
// SCOPE-NOT: DebugScope [[S_AF_B1]]{{$}}
// SCOPE-NOT: DebugScope [[S_AF_B2]]{{$}}
// SCOPE-NOT: DebugScope [[S_AA]]{{$}}
// SCOPE-NOT: DebugScope [[S_BF]]{{$}}
// SCOPE-NOT: DebugScope [[S_F]]{{$}}
// SCOPE-NOT: DebugScope [[S_F_B1]]{{$}}
// SCOPE-NOT: DebugScope [[S_F_B2]]{{$}}
// SCOPE-NOT: DebugScope [[S_A]]{{$}}
// SCOPE: DebugScope [[S_BA]]{{$}}
// SCOPE-NOT: DebugScope [[S_CM]]{{$}}
// SCOPE-NOT: DebugScope [[S_AF]]{{$}}
// SCOPE-NOT: DebugScope [[S_AF_B1]]{{$}}
// SCOPE-NOT: DebugScope [[S_AF_B2]]{{$}}
// SCOPE-NOT: DebugScope [[S_AA]]{{$}}
// SCOPE-NOT: DebugScope [[S_BF]]{{$}}
// SCOPE-NOT: DebugScope [[S_F]]{{$}}
// SCOPE-NOT: DebugScope [[S_F_B1]]{{$}}
// SCOPE-NOT: DebugScope [[S_F_B2]]{{$}}
// SCOPE-NOT: DebugScope [[S_A]]{{$}}
// SCOPE-LABEL: = OpFunction %
