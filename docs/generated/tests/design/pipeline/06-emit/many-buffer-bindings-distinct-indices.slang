//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#backends
//META: doc_section_digest=4c9727df0c383d0b7f12c2f3e71feb78bbba71d1bad80cd7ab19a86ad4cfc825
//META: purpose=Stress: four buffer globals receive four distinct consecutive binding indices in every backend that spells bindings — HLSL `u0..u3`, GLSL `binding = 0..3`, SPIR-V `Binding 0..3`, WGSL `@binding(0..3)`.
//META: intent=stress
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a backend's binding emission scales past the single-resource
// case and does not collide. Four separate buffers are declared and all four
// are written, so none can be dead-code eliminated; each prefix asserts all
// four indices appear. `CHECK-DAG` is used because declaration order in the
// emitted text is not the property under test — index distinctness is. Metal
// spells the same thing as an argument-table slot, `[[buffer(N)]]` on the
// kernel parameter, so it is exercised too; the `[[` is written as a regex
// block because FileCheck otherwise reads it as a variable reference.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute

RWStructuredBuffer<int> bufA;
RWStructuredBuffer<int> bufB;
RWStructuredBuffer<int> bufC;
RWStructuredBuffer<int> bufD;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    bufA[tid.x] = int(tid.x);
    bufB[tid.x] = int(tid.y);
    bufC[tid.x] = int(tid.z);
    bufD[tid.x] = int(tid.x + tid.y + tid.z);
}

// HLSL-DAG: bufA_{{[0-9]+}} : register(u0)
// HLSL-DAG: bufB_{{[0-9]+}} : register(u1)
// HLSL-DAG: bufC_{{[0-9]+}} : register(u2)
// HLSL-DAG: bufD_{{[0-9]+}} : register(u3)

// GLSL-DAG: layout(std430, binding = 0) buffer
// GLSL-DAG: layout(std430, binding = 1) buffer
// GLSL-DAG: layout(std430, binding = 2) buffer
// GLSL-DAG: layout(std430, binding = 3) buffer

// SPIRV-DAG: OpDecorate %bufA Binding 0
// SPIRV-DAG: OpDecorate %bufB Binding 1
// SPIRV-DAG: OpDecorate %bufC Binding 2
// SPIRV-DAG: OpDecorate %bufD Binding 3

// WGSL-DAG: @binding(0) @group(0) var<storage, read_write> bufA_
// WGSL-DAG: @binding(1) @group(0) var<storage, read_write> bufB_
// WGSL-DAG: @binding(2) @group(0) var<storage, read_write> bufC_
// WGSL-DAG: @binding(3) @group(0) var<storage, read_write> bufD_

// METAL-DAG: bufA_{{[0-9]+}} {{\[\[}}buffer(0)
// METAL-DAG: bufB_{{[0-9]+}} {{\[\[}}buffer(1)
// METAL-DAG: bufC_{{[0-9]+}} {{\[\[}}buffer(2)
// METAL-DAG: bufD_{{[0-9]+}} {{\[\[}}buffer(3)
