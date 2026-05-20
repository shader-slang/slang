//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/cross-cutting/core-module.md#what-the-core-module-provides
//META: doc_section_digest=cf6e88293f416086490f900fb6322b3e953eeecf6ea19c2143f0800c759fa6c5
//META: purpose=Boundary: core-module implicit conversion accepts int32_t(MAX) -> float32_t; the cast is preserved in HLSL emit.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: type conversion — int32_t MAX cast to float32_t.
// The doc lists implicit conversions as part of the core-module
// vocabulary. Route through tid so the conversion survives in emit.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

RWStructuredBuffer<float32_t> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int32_t maxInt = 2147483647;
    int32_t v = maxInt - int32_t(tid.x);
    float32_t f = float32_t(v);
    outBuf[tid.x] = f;
}

// HLSL: float(
