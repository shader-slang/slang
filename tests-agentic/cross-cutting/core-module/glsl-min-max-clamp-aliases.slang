//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/cross-cutting/core-module.md#glsl-module
//META: doc_section_digest=2f7ea7343ac21591a146336670d19bda76cac7c8591fc4af2e2833489d070e98
//META: purpose=Boundary: under -allow-glsl, GLSL-flavoured names `min`, `max`, `clamp` resolve and lower to per-target spellings.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: GLSL-named intrinsic surface area — `min`, `max`,
// and `clamp` under -allow-glsl. The doc covers the GLSL module as
// loaded conditionally; these three are the most common arithmetic
// helpers it provides.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute -allow-glsl

RWStructuredBuffer<vec3> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    vec3 a = vec3(float(tid.x), float(tid.y), float(tid.z));
    vec3 b = vec3(2.0, 1.0, 4.0);
    vec3 c = min(a, b);
    vec3 d = max(a, b);
    vec3 e = clamp(a, vec3(0.0), vec3(2.0));
    outBuf[tid.x] = c + d + e;
}

// GLSL: min(
// GLSL: max(
// GLSL: clamp(
