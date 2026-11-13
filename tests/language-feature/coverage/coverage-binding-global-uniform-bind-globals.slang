// Regression test for shader-slang/slang#14013.
//
// `-fvk-bind-globals 5 9` places the default constant buffer for the global uniform
// `value` at set 9, binding 5, so set 0, binding 0 is free for the coverage buffer,
// while the buffer's own slot still collides.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry computeMain -fvk-bind-globals 5 9 -trace-coverage-binding 0 0
//TEST:SIMPLE(filecheck=COLLIDE):-target spirv-asm -stage compute -entry computeMain -fvk-bind-globals 5 9 -trace-coverage-binding 5 9

// CHECK-DAG: OpDecorate %__slang_coverage Binding 0
// CHECK-DAG: OpDecorate %__slang_coverage DescriptorSet 0
// CHECK-DAG: OpDecorate %globalParams Binding 5
// CHECK-DAG: OpDecorate %globalParams DescriptorSet 9

// COLLIDE: error[E45101]: `-trace-coverage-binding` collides with an existing parameter

[[vk::binding(3)]] RWStructuredBuffer<float> result;
uniform float value;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0)
    {
        result[0] = value;
    }
}
