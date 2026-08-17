//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=6e338f870e1f336f86d9ce297f0955e56a8c161eb5a129e60e2a396aa3aa18b3
//META: purpose=Expansion: an RWTexture3D is written and read via the uint3 subscript; HLSL emit retains the RWTexture3D type and binds it to a `u` register.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// RWTexture3D is the UAV form of Texture3D. Subscript is uint3.
// HLSL emit preserves the type spelling and uses the `u` register
// class.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWTexture3D<float4> tex;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    tex[uint3(0, 0, 0)] = float4(1.0, 0.0, 0.0, 1.0);
    outBuf[tid.x] = tex[uint3(0, 0, 0)];
}

// CHECK: RWTexture3D{{.*}} tex_{{[0-9]+}} : register(u{{[0-9]+}})
