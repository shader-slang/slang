//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/cross-cutting/core-module.md#core-module
//META: doc_section_digest=2bfd479afe0dc76144283c14d90b22a6ac0fd8d03be7907d3412c1e2f3944a4b
//META: purpose=Boundary: the HLSL meta-module's `normalize` intrinsic accepts a zero-vector argument; the HLSL backend preserves the `normalize(` call (runtime result is implementation-defined, not asserted here).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: numeric edge — normalize of the zero vector. The
// doc lists `normalize`-class intrinsics as HLSL-meta-module members
// but says nothing about zero-vector semantics (see doc-gap note in
// BUNDLE.md). We assert only front-end acceptance and emit-side
// preservation of the call.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

RWStructuredBuffer<float3> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float zero_scale = float(tid.x) * 0.0;
    float3 zerovec = float3(zero_scale, zero_scale, zero_scale);
    outBuf[tid.x] = normalize(zerovec);
}

// HLSL: normalize(
