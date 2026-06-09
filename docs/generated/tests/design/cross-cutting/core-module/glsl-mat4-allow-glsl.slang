//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#glsl-module
//META: doc_section_digest=2f7ea7343ac21591a146336670d19bda76cac7c8591fc4af2e2833489d070e98
//META: purpose=Boundary: the GLSL module provides `mat4` (4x4 matrix alias) under -allow-glsl and the per-target lowering preserves matrix-vector multiply.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: matrix dim via GLSL alias — `mat4` from the GLSL
// module under -allow-glsl, exercising the largest common square
// matrix dim.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute -allow-glsl
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute -allow-glsl

RWStructuredBuffer<vec4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    mat4 m = mat4(1.0);
    vec4 v = vec4(float(tid.x), float(tid.y), float(tid.z), 1.0);
    outBuf[tid.x] = m * v;
}

// HLSL: float4x4
// GLSL: mat4
