// Verifies that `-trace-coverage-binding <index> <space>` steers the
// synthesized `__slang_coverage` buffer to a user-specified slot on
// the SPIR-V target. The HLSL `register(uN, spaceM)` semantic is
// translated by parameter binding into SPIR-V `Binding N` and
// `DescriptorSet M` decorations.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry computeMain -trace-coverage-binding 7 3

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0)
    {
        int x = 1 + 2;
    }
}

// The synthesized coverage buffer should land at exactly DescriptorSet 3,
// Binding 7. If the option is ignored or maps onto a different (set,
// binding) pair, these checks catch the regression.
//CHECK-DAG: OpDecorate %__slang_coverage Binding 7
//CHECK-DAG: OpDecorate %__slang_coverage DescriptorSet 3
