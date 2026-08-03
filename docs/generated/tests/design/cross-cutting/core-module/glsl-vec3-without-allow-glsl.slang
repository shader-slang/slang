//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#glsl-module
//META: doc_section_digest=af07acb9006b2e6bd2486c2a95f1ca55cef989b24178836276686391dc055ee3
//META: purpose=Negative: the GLSL builtin module is loaded only when the session opts in (SlangGlobalSessionDesc::enableGLSL); without -allow-glsl, `vec3` is an undefined identifier in Slang source.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the GLSL builtin module is opt-in, not loaded by default:
// its names exist only in sessions that enable GLSL support. We
// compile Slang source that uses `vec3` without -allow-glsl; the
// CHECK pins the undefined-identifier diagnostic code E30015, which
// only fires if the module's names are absent from lookup.

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
