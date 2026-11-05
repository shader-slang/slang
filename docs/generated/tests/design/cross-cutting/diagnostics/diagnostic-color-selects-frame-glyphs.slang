//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#source-locations-and-message-rendering
//META: doc_section_digest=f952036c11ff9b8c1d73b32cb9eb2592525bdd2e44c5b243e4cfdec0a9352108
//META: purpose=-diagnostic-color selects the renderer's frame glyphs as well as its colour, so `never` draws the ASCII frame and `always` draws the Unicode box-drawing frame.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The colour setting is not only about colour: absent an explicit host
// unicode setting, the same predicate picks the glyphs the renderer draws
// the diagnostic frame with. One undefined identifier is compiled twice.
// Under `-diagnostic-color never` the ASCII directives are asserted -
// the `-->` location arrow, the `|` gutter and the `^` caret run - and
// the Unicode glyphs are asserted absent. Under `-diagnostic-color
// always` the assertions swap: the `-->` arrow must be gone and the
// `╭╼` / `│` / `━` box-drawing forms must be present. Asserting each
// mode's absence as well as its presence is what makes this a test of
// the selection rather than of one rendering. The `[E30015]` lines pin
// the failure to the undefined identifier, so that `exit-code=-1` cannot
// pass on an unrelated error. They match only the bracketed code because
// `always` mode wraps the severity word in colour escapes, and they are
// plain CHECK lines ahead of the `-NOT` lines because a `-DAG` match on
// the header line would shrink each `-NOT` region to the text before it.

//TEST:SIMPLE(filecheck=ASCII,exit-code=-1):-target hlsl -entry main -stage compute -diagnostic-color never
//TEST:SIMPLE(filecheck=UNICODE,exit-code=-1):-target hlsl -entry main -stage compute -diagnostic-color always

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = notDeclaredAnywhere;
}

// ASCII: [E30015]
// ASCII-NOT: {{╭╼}}
// ASCII-DAG: --> {{.*}}diagnostic-color-selects-frame-glyphs.slang
// ASCII-DAG: {{\^\^\^\^}}

// UNICODE: [E30015]
// UNICODE-NOT: -->
// UNICODE-DAG: {{╭╼}}{{.*}}diagnostic-color-selects-frame-glyphs.slang
// UNICODE-DAG: {{━━━━}}
