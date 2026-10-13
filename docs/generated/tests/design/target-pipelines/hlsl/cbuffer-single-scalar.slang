//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=c7c97cf6f5a8dc2bac27a37c7292d84bc70e3e1764ddec58018711cf9fd606ce
//META: purpose=Smallest-shape boundary: a cbuffer with a single scalar field still emits the canonical struct-then-cbuffer pair.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: a cbuffer with only one field — the minimum
// non-empty layout. The lifted parameter-group struct should
// still appear, and the cbuffer should still wrap it.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

cbuffer Params {
    float onlyScalar;
};
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = onlyScalar;
}

// CHECK: struct SLANG_ParameterGroup_Params{{.*}}
// CHECK: float onlyScalar_{{[0-9]+}};
// CHECK: cbuffer Params_{{[0-9]+}} : register(b{{[0-9]+}})
