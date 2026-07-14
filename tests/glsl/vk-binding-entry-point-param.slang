//TEST:SIMPLE(filecheck=GLSL): -allow-glsl -target glsl -entry main -stage compute

// `doesTargetSupportVkBindingOnEntryPointParameters` is true for all Khronos
// targets, which includes the GLSL emit path (separate from SPIR-V). Verify the
// GLSL backend honors `[[vk::binding(b, s)]]` on an entry-point resource
// parameter as `layout(binding = b, set = s)`, with the implicit sibling
// allocated into the default set 0.

[shader("compute")]
[numthreads(1, 1, 1)]
void main(
    [[vk::binding(3, 2)]]
    RWStructuredBuffer<float> explicitB,
    RWStructuredBuffer<float> implicitC)
{
    explicitB[0] = implicitC[0];
}

// GLSL-DAG: layout({{.*}}binding = 3, set = 2)
// GLSL-DAG: layout({{.*}}binding = 0){{.*}}buffer
