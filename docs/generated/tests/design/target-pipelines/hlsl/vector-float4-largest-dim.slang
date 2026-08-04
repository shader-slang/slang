//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=619da2387b17d1b514e8cb1b82a03f336653b7502413b2205a6fc4753c4eeacd
//META: purpose=Boundary: largest fixed-width vector (float4) is spelled `float4` in HLSL emit, distinct from GLSL `vec4`.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: max-documented vector dimension (4). The
// HLSLSourceEmitter must still use the `float<N>` spelling for
// the four-element vector.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = float4(0.0, 1.0, 2.0, 3.0);
}

// CHECK: float4
