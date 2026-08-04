//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#eliminatephis-with-default-options
//META: doc_section_digest=2b0ff9304692be73626bf7bd3c50f028f4c346a134dc0949d2343d82c090c772
//META: purpose=Conditional struct selection (non-vector composite select) lowers to an if/else block assigning a local of the struct type; the HLSL-only legalizeNonVectorCompositeSelect arm does not run for Metal but eliminatePhis already produces the same shape.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `legalizeNonVectorCompositeSelect` is HLSL-only and does not
// run for Metal, but Metal's default phi elimination already
// turns `cond ? structA : structB` into an if/else write-back to
// a local of the struct type.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

struct Vec
{
    float4 v;
};

RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Vec a;
    a.v = float4(1, 0, 0, 1);
    Vec b;
    b.v = float4(0, 1, 0, 1);
    Vec c = (tid.x > 0u) ? a : b;
    outBuf[tid.x] = c.v;
}

// CHECK: Vec_{{[0-9]+}} c_{{[0-9]+}};
// CHECK: if(
// CHECK: else
