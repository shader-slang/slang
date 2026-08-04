//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#what-the-core-module-provides
//META: doc_section_digest=8a82ae360bcd088373acbb80719dc095678434371e751937b8feacab2eae3d90
//META: purpose=Boundary: `float32_t` accepts NaN (0.0/0.0) and lowers per target without diagnostic.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: float NaN — the core-module `float32_t` typedef
// must accept a NaN-producing expression.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

RWStructuredBuffer<float32_t> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float32_t nan_val = 0.0 / 0.0;
    outBuf[tid.x] = nan_val + float32_t(tid.x);
}

// HLSL: void main
