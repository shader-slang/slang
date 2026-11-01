// `clip` on a matrix uses the core module's `any(x < T(0))` fallback, a matrix-scalar comparison.
// On SPIR-V and GLSL that comparison used to crash slangc in matrix legalization (see
// matrix-scalar-comparison.slang). `clip` is not available for Metal fragment shaders, so this
// entry point has its own file.

//TEST:SIMPLE(filecheck=CLIP): -target spirv-asm -entry fragmentMain -stage fragment
//TEST:SIMPLE(filecheck=CLIP): -target glsl -entry fragmentMain -stage fragment

// CLIP-NOT: {{error\[E[0-9]+\]}}
// CLIP: {{OpKill|OpTerminateInvocation|OpDemoteToHelperInvocation|discard}}

[shader("fragment")]
float4 fragmentMain(float4 pos: SV_Position) : SV_Target
{
    clip(float4x4(pos.x - 1.0));
    return pos;
}
