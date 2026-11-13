//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=affc65088b4faddcbee9b222f5a894089c99cb8eee13a5f0c0bac210c0fd5c78
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
