//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#glsl-module
//META: doc_section_digest=72709ab05fe0048310a93cb48fcbdc8ad10b25203945acdda232920d909f6344
//META: purpose=The GLSL builtin module is opt-in: without the GLSL module enabled, vec3 is an undefined identifier.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the GLSL module is loaded only when the session asks for
// it, unlike the core and HLSL meta-modules which are always in
// scope. The same `vec3` spelling that compiles with the GLSL module
// enabled is compiled here without it; the CHECK pins the
// undefined-identifier code E30015 at the declaration, which can only
// fire if the module's names are absent from lookup entirely.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    vec3 v = vec3(float(tid.x), 1.0, 2.0);
/*CHECK:
    ^^^^ undefined identifier 'vec3'.
*/
    // The error code stays pinned separately so a renumbering fails too.
    //CHECK: E30015
    outBuf[tid.x] = v.x;
}
