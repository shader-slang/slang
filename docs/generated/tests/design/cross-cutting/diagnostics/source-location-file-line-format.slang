//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T12:30:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#source-locations-and-message-rendering
//META: doc_section_digest=d4c1621bec12a7398cb1a9674411d74b7313361b2b6d60447dbaae79eb52f58e
//META: purpose=The sink decodes SourceLoc via SourceManager and renders the location with the test file's path so consumers can navigate to the offending site.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc claims the sink uses SourceManager to decode SourceLoc into
// path information for the user. Verify that the rendered diagnostic
// names this source file so a downstream tool can open it.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int a = nonexistent;
}

// CHECK: source-location-file-line-format.slang
// CHECK: undefined identifier
