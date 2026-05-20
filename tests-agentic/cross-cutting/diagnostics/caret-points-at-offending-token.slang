//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T12:30:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#source-locations-and-message-rendering
//META: doc_section_digest=d4c1621bec12a7398cb1a9674411d74b7313361b2b6d60447dbaae79eb52f58e
//META: purpose=The sink uses SourceManager to decode SourceLoc into file:line:column and renders a caret aligned to the offending token's column.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verify caret column alignment. The undefined identifier 'b' sits at
// column 13 on the line below; the DIAGNOSTIC_TEST runner enforces
// caret-column alignment, so this test would fail if the rendered
// column drifted from the offending token.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
void main()
{
    int a = b;
//CHECK:    ^ undefined identifier
//CHECK:    ^ undefined identifier 'b'.
}
