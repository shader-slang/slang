//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T12:30:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#severity-levels
//META: doc_section_digest=6c1c702cf80ed3f86a13f8071f7debe50ebc4cb725e9ba9101ed110bca3e30e0
//META: purpose=A diagnostic emitted at Severity::Error renders with the user-facing name "error".
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Use of an undefined identifier is a known Severity::Error diagnostic
// (undefined-identifier, E30015). Verify the rendered severity word is
// the doc-claimed "error" by FileChecking that word on the caret line.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
void main()
{
    int a = b;
//CHECK:    ^ error
//CHECK:    ^ undefined identifier
}
