//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Pins that the explicit `Std140DataLayout` / `Std430DataLayout` / `ScalarDataLayout` arguments of `StructuredBuffer` select the matching GLSL block layout qualifier, and that `ScalarDataLayout` pulls in `GL_EXT_scalar_block_layout`.
//META: intent=characterization
//META: covers=source/slang/slang-emit-glsl.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes the data-layout switch in `GLSLSourceEmitter::_emitGLSLStructuredBuffer`'s
// block header. A `StructuredBuffer<T, L>` carries its layout as a second type
// argument; the GLSL back end maps each one onto the block qualifier GLSL spells
// (`std140` / `std430` / `scalar`) and, for the scalar case only, additionally
// requires `GL_EXT_scalar_block_layout` because plain GLSL 450 has no `scalar`
// layout. Each CHECK-DAG pins one arm of that mapping together with the buffer it
// belongs to, so a mis-mapped layout shows up as a failed match rather than as a
// silently different qualifier.
// The same shader also pins two neighbouring decisions in that header: a
// read-only `StructuredBuffer` gets the `readonly` qualifier while the writable
// `RWStructuredBuffer` does not, and a `globallycoherent` buffer is prefixed with
// GLSL's `coherent` memory qualifier.
//
// SPIR-V is pinned for contrast: it has no block-layout keyword at all, expressing
// the same information as explicit `ArrayStride` / `Offset` decorations, and it
// spells `globallycoherent` as an `OpDecorate ... Coherent` on the variable.
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute

StructuredBuffer<float4, Std140DataLayout> std140Buffer;
StructuredBuffer<float4, Std430DataLayout> std430Buffer;
StructuredBuffer<float4, ScalarDataLayout> scalarBuffer;
globallycoherent RWStructuredBuffer<float4> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = std140Buffer[tid.x] + std430Buffer[tid.x] + scalarBuffer[tid.x];
}

// `scalar` is the only layout that needs an extension.
//GLSL-DAG: #extension GL_EXT_scalar_block_layout : require
// One qualifier per declared layout; the read-only buffers also get `readonly`.
//GLSL-DAG: layout(std140, binding = 0) readonly buffer
//GLSL-DAG: layout(std430, binding = 1) readonly buffer
//GLSL-DAG: layout(scalar, binding = 2) readonly buffer
// `globallycoherent` becomes GLSL's `coherent`, and the writable buffer is not `readonly`.
//GLSL-DAG: coherent layout(std430, binding = 3) buffer

// SPIR-V carries the layout as explicit strides/offsets instead of a keyword.
//SPV-DAG: OpDecorate %_runtimearr_v4float ArrayStride 16
//SPV-DAG: OpMemberDecorate %RWStructuredBuffer 0 Offset 0
//SPV-DAG: OpDecorate %outputBuffer Coherent
