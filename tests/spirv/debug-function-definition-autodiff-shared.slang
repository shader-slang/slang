//TEST:SIMPLE(filecheck=CHECK):-target spirv -g2 -O0 -emit-spirv-directly -stage compute -entry computeMain

// A direct call to f and reverse-mode autodiff's generated s_apply_f body can share one
// DebugFunction record: copyDebugInfo copies the decoration without remapping its module-global
// operand. The record must have at most one DebugFunctionDefinition.
//
// The loop forces the reverse-mode primal/propagate split. Calling f directly as well keeps two
// bodies that share its record. Address-space specialization clones s_bwdProp_f's debug scopes,
// so the backward call alone no longer exercises record-level deduplication.

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

// f and s_apply_f share this record. Capture instruction ids so the embedded source cannot
// satisfy these checks, then reject a second definition for the same record.
// CHECK: [[F_STR:%[0-9]+]] = OpString "f"
// CHECK: [[F_REC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[F_STR]]
// CHECK-COUNT-1: OpExtInst %void %{{[0-9]+}} DebugFunctionDefinition [[F_REC]]
// CHECK-NOT: OpExtInst %void %{{[0-9]+}} DebugFunctionDefinition [[F_REC]]
