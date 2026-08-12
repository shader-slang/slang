//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute

// An entry point may mix an explicitly `[[vk::binding]]`-bound resource parameter
// with a scalar `uniform` parameter that aggregates into a push constant. The
// per-field completion path must honor the explicit descriptor binding while
// leaving the push-constant promotion intact (it must not be pulled into a
// descriptor set). This is the companion of `push-constant-space.slang`, which
// covers the no-explicit-binding case.

[numthreads(1, 1, 1)]
[shader("compute")]
void main(
    [[vk::binding(3, 2)]]
    RWStructuredBuffer<float> explicitB,
    uniform uint scalarU)
{
    explicitB[0] = scalarU;
}

// The explicit resource gets its requested (binding 3, set 2).
// CHECK-DAG: OpDecorate %entryPointParams_explicitB Binding 3
// CHECK-DAG: OpDecorate %entryPointParams_explicitB DescriptorSet 2

// The scalar uniform becomes a push constant, not a descriptor.
// CHECK: %entryPointParams = OpVariable %{{.*}} PushConstant
