//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/cross-cutting/core-module.md#glsl-module
//META: doc_section_digest=2f7ea7343ac21591a146336670d19bda76cac7c8591fc4af2e2833489d070e98
//META: purpose=Negative: the GLSL module is target-conditional; without -allow-glsl, `vec3` is an undefined identifier in Slang source.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative: the doc states the GLSL module loads conditionally —
// when compiling GLSL or when the user asks for GLSL-flavoured names
// from Slang code. Without -allow-glsl, `vec3` in Slang source must
// not resolve.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    vec3 v = vec3(float(tid.x), 1.0, 2.0);
    //CHECK: undefined identifier 'vec3'.
// CHECK: E30015
    outBuf[tid.x] = v.x;
}
