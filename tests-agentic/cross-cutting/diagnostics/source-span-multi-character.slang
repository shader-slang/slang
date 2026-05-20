//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T12:30:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#source-locations-and-message-rendering
//META: doc_section_digest=d4c1621bec12a7398cb1a9674411d74b7313361b2b6d60447dbaae79eb52f58e
//META: purpose=The sink uses SourceManager to decode source locations and renders carets that span the full offending token, not just the start.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says SourceLoc decodes into file:line:column and the sink
// renders carets aligned to the source. Verify the rendered caret
// span covers the entire offending identifier (a multi-character
// run), not merely its starting column.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
void main()
{
    int a = longUndeclaredName;
//CHECK:    ^^^^^^^^^^^^^^^^^^ undefined identifier
//CHECK:    ^^^^^^^^^^^^^^^^^^ undefined identifier 'longUndeclaredName'.
}
