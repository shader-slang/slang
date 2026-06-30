//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry main -stage compute

// `doesTargetSupportVkBindingOnEntryPointParameters` enables `[[vk::binding]]` on
// entry-point parameters for WGPU as well as Khronos targets. Verify the WGSL
// backend emits the requested `@binding(b) @group(s)` for an entry-point resource
// parameter, with the implicit sibling allocated into the default group 0.

[shader("compute")]
[numthreads(1, 1, 1)]
void main(
    [[vk::binding(3, 2)]]
    RWStructuredBuffer<float> explicitB,
    RWStructuredBuffer<float> implicitC)
{
    explicitB[0] = implicitC[0];
}

// WGSL-DAG: @binding(3) @group(2){{.*}}explicitB
// WGSL-DAG: @binding(0) @group(0){{.*}}implicitC
