//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#source-locations-and-message-rendering
//META: doc_section_digest=c0e97396ea139ceab54212053b6d443692d19f0faf2847c4dc505373ae8c4861
//META: purpose=The sink renders the decoded location as a `file:line:column` triple naming the source file.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the sink decodes the SourceLoc through SourceManager into a
// location that names the originating file. CHECK asserts this file's name
// appears alongside the undefined-identifier message, which is only possible
// if the path component of the decoded triple survives rendering.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int a = nonexistent;
}

// CHECK: source-location-file-line-format.slang
// CHECK: undefined identifier
