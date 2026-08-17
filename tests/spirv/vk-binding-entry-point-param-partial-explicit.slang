//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute

// When an entry point mixes an explicitly `[[vk::binding]]`-bound parameter with
// an implicitly-bound one of the same descriptor kind, the implicit parameter
// still needs a default space, so `_calcNeedsDefaultSpace` must not skip it just
// because some sibling is explicit. The explicit parameter keeps its requested
// (binding 3, set 2); the implicit parameter and the global parameter are
// allocated into the default space 0 without colliding.

RWStructuredBuffer<float> gData;

[numthreads(1, 1, 1)]
[shader("compute")]
void main(
    [[vk::binding(3, 2)]]
    RWStructuredBuffer<float> explicitB,
    RWStructuredBuffer<float> implicitC)
{
    gData[0] = explicitB[0] + implicitC[0];
}

// CHECK-DAG: OpDecorate %gData Binding 0
// CHECK-DAG: OpDecorate %gData DescriptorSet 0
// CHECK-DAG: OpDecorate %entryPointParams_explicitB Binding 3
// CHECK-DAG: OpDecorate %entryPointParams_explicitB DescriptorSet 2
// CHECK-DAG: OpDecorate %entryPointParams_implicitC Binding 1
// CHECK-DAG: OpDecorate %entryPointParams_implicitC DescriptorSet 0
