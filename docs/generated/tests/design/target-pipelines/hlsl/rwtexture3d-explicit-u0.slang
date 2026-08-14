//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=6e338f870e1f336f86d9ce297f0955e56a8c161eb5a129e60e2a396aa3aa18b3
//META: purpose=Boundary: an RWTexture3D bound `register(u0)` (lower-edge UAV index) preserves its explicit `u0` annotation through HLSL emit.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: lowest legal `u` register index (0) on an
// RWTexture3D. Slang carries the explicit register annotation
// through to the HLSL declaration verbatim.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWTexture3D<float4> tex : register(u0);
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    tex[uint3(0, 0, 0)] = float4(1.0, 0.0, 0.0, 1.0);
    outBuf[tid.x] = float4(1.0);
}

// CHECK: RWTexture3D{{.*}} tex_{{[0-9]+}} : register(u0)
