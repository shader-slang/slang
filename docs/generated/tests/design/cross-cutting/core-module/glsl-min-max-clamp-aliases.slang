//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#glsl-module
//META: doc_section_digest=af07acb9006b2e6bd2486c2a95f1ca55cef989b24178836276686391dc055ee3
//META: purpose=Boundary: under -allow-glsl, GLSL-flavoured names `min`, `max`, `clamp` resolve and lower to per-target spellings.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: GLSL-named intrinsic surface area — `min`, `max`,
// and `clamp` under -allow-glsl. The GLSL builtin module is opt-in;
// once enabled, these three common arithmetic helpers resolve and
// the GLSL backend preserves their spellings.

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
