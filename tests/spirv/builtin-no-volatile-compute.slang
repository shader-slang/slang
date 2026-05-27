// builtin-no-volatile-compute.slang
//
// Regression test for shader-slang/slang#10528.
// `Volatile` on subgroup builtins is a raytracing-stage requirement;
// compute shaders must not get the extra decoration.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute -emit-spirv-directly

RWStructuredBuffer<uint> output;

// Single-entry-point shader; no use of `Volatile` should appear in the
// SPIR-V at all (neither as a variable decoration nor as a per-access
// mask), so a blanket `CHECK-NOT: Volatile` is the strongest form.
// CHECK: OpDecorate %{{[A-Za-z0-9_]+}} BuiltIn SubgroupLocalInvocationId
// CHECK-NOT: Volatile
[numthreads(64, 1, 1)]
[shader("compute")]
void computeMain()
{
    output[0] = WaveGetLaneIndex();
}
