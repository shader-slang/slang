// Regression guard: atomics on true GLSL UBOs (uniform buffers) must be rejected.
// A UBO uses Uniform storage class with Block decoration (not BufferBlock), which
// makes its members read-only — atomics are therefore rejected at the l-value check
// (E30047), before the atomic-destination validator (E41403) is even reached.
// This test confirms the fix in gh-8959 (which accepts Uniform+BufferBlock for SSBOs)
// does not accidentally allow atomics on read-only UBOs.
// See https://github.com/shader-slang/slang/issues/8959
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-profile glsl_460 -target spirv
#version 460

layout(std140, binding = 0) uniform uboBuffer
{
    uint data;
};

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    // UBO members are read-only; atomicAdd on one must be rejected as not an l-value.
    // CHECK: error[E30047]
    atomicAdd(data, 1u);
}
