//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T12:30:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#severity-levels
//META: doc_section_digest=6c1c702cf80ed3f86a13f8071f7debe50ebc4cb725e9ba9101ed110bca3e30e0
//META: purpose=A diagnostic emitted at Severity::Warning renders with the user-facing name "warning".
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The comma-operator-used-in-expression warning (W41024) fires when a
// comma operator appears in a non-side-effect-only context. Verify the
// rendered severity word is the doc-claimed "warning".

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
int main()
{
    int x = (1, 2);
//CHECK:      ^ warning
//CHECK:      ^ comma operator used in expression
    return x;
}
