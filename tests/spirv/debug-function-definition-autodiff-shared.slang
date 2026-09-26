//TEST:SIMPLE(filecheck=CHECK):-target spirv -g2 -O0 -emit-spirv-directly -stage compute -entry computeMain

// Regression guard for record-level dedup of DebugFunctionDefinition (shader-slang/slang#13235).
//
// Reverse-mode autodiff's copyDebugInfo clones the debug-func decoration onto the functions it
// generates (s_apply_ / s_bwdProp_), and the module-global IRDebugFunction operand is not
// remapped, so several generated OpFunctions share one DebugFunction record. The emitter must
// emit at most one DebugFunctionDefinition per record (a DebugFunction binds to a single
// definition); deduping on the OpFunction body instead would emit one definition per shared body.
// Neither the rest of tests/spirv nor spirv-val distinguishes the two shapes, so this guards the
// record-level dedup explicitly. The loop forces the reverse-mode primal/propagate split that
// produces the shared-record bodies.

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

// f's DebugFunction record is shared by the generated s_apply_f / s_bwdProp_f bodies, so it must
// have a single DebugFunctionDefinition. The record and name ids are captured (not source text) so
// the -g2 embedded source cannot satisfy the match; the count-one match plus the following negative
// match pin it to exactly one definition.
// CHECK: [[F_STR:%[0-9]+]] = OpString "f"
// CHECK: [[F_REC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[F_STR]]
// CHECK-COUNT-1: OpExtInst %void %{{[0-9]+}} DebugFunctionDefinition [[F_REC]]
// CHECK-NOT: OpExtInst %void %{{[0-9]+}} DebugFunctionDefinition [[F_REC]]
