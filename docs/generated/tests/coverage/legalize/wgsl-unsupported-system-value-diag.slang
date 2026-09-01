//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Characterizes that a system-value semantic the WGSL varying-parameter legalizer marks unsupported is reported as E55202 naming the lowercased semantic, rather than silently dropped.
//META: intent=characterization
//META: covers=source/slang/slang-ir-legalize-varying-params.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the `isUnsupported` arm of
// `LegalizeWGSLEntryPointContext::getSystemValueInfo` and the shared
// `reportUnsupportedSystemAttribute` it funnels into
// (slang-ir-legalize-varying-params.cpp:4804 and :3282). WGSL currently has
// no clip-distance builtin, so the table marks `SV_ClipDistance`
// unsupported rather than mapping it. What this test pins is that the
// unsupported marking produces a real user-facing diagnostic naming the
// semantic (in its lowercased internal spelling) at the declaration site,
// which is the observable difference from the semantic being ignored and
// silently emitted as an ordinary location.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target wgsl -entry vertMain -stage vertex

struct VOut
{
    float4 pos : SV_Position;
    float clip[1] : SV_ClipDistance;
//CHECK: E55202
//CHECK: system value semantic 'sv_clipdistance' is not supported for the current target.
}

[shader("vertex")]
VOut vertMain()
{
    VOut o;
    o.pos = float4(0);
    o.clip[0] = 0;
    return o;
}
