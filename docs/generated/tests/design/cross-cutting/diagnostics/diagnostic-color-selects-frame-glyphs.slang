//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#source-locations-and-message-rendering
//META: doc_section_digest=b56b816820af1c15450b34e955f5435969bcf8f2936c6dbb068f8ab21ec3dca3
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
// the selection rather than of one rendering.

//TEST:SIMPLE(filecheck=ASCII,exit-code=-1):-target hlsl -entry main -stage compute -diagnostic-color never
//TEST:SIMPLE(filecheck=UNICODE,exit-code=-1):-target hlsl -entry main -stage compute -diagnostic-color always

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = notDeclaredAnywhere;
}

// ASCII-NOT: {{╭╼}}
// ASCII-DAG: --> {{.*}}diagnostic-color-selects-frame-glyphs.slang
// ASCII-DAG: {{\^\^\^\^}}

// UNICODE-NOT: -->
// UNICODE-DAG: {{╭╼}}{{.*}}diagnostic-color-selects-frame-glyphs.slang
// UNICODE-DAG: {{━━━━}}
