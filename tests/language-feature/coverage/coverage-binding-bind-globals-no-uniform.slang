// Regression test for shader-slang/slang#14013.
//
// `-fvk-bind-globals 5 9` reserves set 9, binding 5 for the default constant buffer, but no
// global here has ordinary data, so there is no constant buffer and nothing occupies that
// slot. The coverage buffer can take it, and the resource globals stay in set 0.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry computeMain -fvk-bind-globals 5 9 -trace-coverage-binding 5 9

// CHECK-DAG: OpDecorate %__slang_coverage Binding 5
// CHECK-DAG: OpDecorate %__slang_coverage DescriptorSet 9
// CHECK-DAG: OpDecorate %result Binding 0
// CHECK-DAG: OpDecorate %result DescriptorSet 0

RWStructuredBuffer<float> result;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0)
    {
        result[0] = 1;
    }
}
