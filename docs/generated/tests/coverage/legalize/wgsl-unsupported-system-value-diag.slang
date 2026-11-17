//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-02T10:45:10+00:00
//META: source_commit=6815ed82d5e8c95b0e1258ca2eba0ca9c7825d86
//META: purpose=Characterizes that a system-value semantic the WGSL varying-parameter legalizer marks unsupported is reported as E55202 naming the lowercased semantic, rather than silently dropped.
//META: intent=characterization
//META: covers=source/slang/slang-ir-legalize-varying-params.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the `isUnsupported` arm of
// `LegalizeWGSLEntryPointContext::getSystemValueInfo` and the shared
// `reportUnsupportedSystemAttribute` it funnels into
// (slang-ir-legalize-varying-params.cpp). WGSL maps `SV_ClipDistance` to its
// `clip_distances` builtin but has no cull-distance builtin, so the table marks
// `SV_CullDistance` unsupported rather than mapping it. What this test pins is that the
// unsupported marking produces a real user-facing diagnostic naming the
// semantic (in its lowercased internal spelling) at the declaration site,
// which is the observable difference from the semantic being ignored and
// silently emitted as an ordinary location.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target wgsl -entry vertMain -stage vertex

struct VOut
{
    float4 pos : SV_Position;
    float cull[1] : SV_CullDistance;
//CHECK: E55202
//CHECK: system value semantic 'sv_culldistance' is not supported for the current target.
}

[shader("vertex")]
VOut vertMain()
{
    VOut o;
    o.pos = float4(0);
    o.cull[0] = 0;
    return o;
}
