//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#what-the-core-module-provides
//META: doc_section_digest=cf6e88293f416086490f900fb6322b3e953eeecf6ea19c2143f0800c759fa6c5
//META: purpose=Boundary: round-trip float32_t -> half -> float32_t is accepted by the front-end; both cast directions appear in HLSL emit.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: type conversion — half <-> float32_t round trip.
// The doc lists `half` and `float` in the core-module's scalar
// vocabulary. We do not assert specific precision behaviour, only
// that the cast chain compiles and survives emit.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

RWStructuredBuffer<float32_t> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float32_t f = float32_t(tid.x) * 2.5;
    half h = half(f);
    float32_t back = float32_t(h);
    outBuf[tid.x] = back;
}

// HLSL: half(
// HLSL: float(
