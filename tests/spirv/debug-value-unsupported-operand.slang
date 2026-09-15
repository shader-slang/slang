// Companion to debug-resource-aliases.slang, guarding the value-side check in SPIR-V legalization
// (canUseAsSPIRVDebugValueOperand). A DebugVar may be created for a supported source-level type,
// but the concrete lowered value bound to it must also be a representable OpDebugValue operand.
//
// A combined texture-sampler is an OpTypeSampledImage value, which SPIR-V does not permit as an
// OpDebugValue operand. Here the value is an OpLoad of the combined-image-sampler global (its result
// type is OpTypeSampledImage); the guard matches by type, so an OpSampledImage result is covered
// equally. The debug variable is left without a current location rather than binding such a value.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute -g2 -O0 -emit-spirv-directly

[[vk::binding(0)]]
Sampler2D<float4> combinedGlobal;

[[vk::binding(1)]]
RWStructuredBuffer<float> results;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    // A combined texture-sampler alias: the bound value is a combined sampled image.
    let combinedAlias = combinedGlobal;
    float2 uv = (float2(threadId.xy) + 0.5) / 64.0;
    results[threadId.x] = combinedAlias.SampleLevel(uv, 0.0).x;
}

// combinedAlias is still a named source variable with a DebugLocalVariable. The bare closing quote
// after the name discriminates the real OpString from the -g2 source blob (which escapes quotes).
// CHECK-DAG: %[[CMBNAME:[0-9]+]] = OpString "combinedAlias"
// CHECK-DAG: %[[CMBVAR:[A-Za-z0-9_]+]] = OpExtInst %void %{{[0-9]+}} DebugLocalVariable %[[CMBNAME]]

// ...but no DebugValue binds the unsupported combined sampled-image value to it. This CHECK-NOT sits
// after the DebugLocalVariable above, spanning the function body where any DebugValue would appear.
// CHECK-NOT: DebugValue %[[CMBVAR]]
