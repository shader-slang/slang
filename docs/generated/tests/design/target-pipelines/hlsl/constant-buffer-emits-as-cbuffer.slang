//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:16:11+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=03d6fe7b8f4502160d35cd7e0f89a3145a7dfe7a6680c44cb1092265f9fe4de7
//META: purpose=ConstantBuffer<T> at module scope emits as cbuffer X : register(bN) in HLSL.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Slang's `ConstantBuffer<T>` lowers to the HLSL `cbuffer X :
// register(bN)` block — semantically identical to a top-level
// `cbuffer` declaration.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

struct Block { float a; int b; };
ConstantBuffer<Block> cb;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = cb.a * float(cb.b);
}

// CHECK: cbuffer cb_{{[0-9]+}} : register(b{{[0-9]+}})
