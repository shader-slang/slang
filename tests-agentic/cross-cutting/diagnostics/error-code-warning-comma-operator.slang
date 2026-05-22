//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T12:30:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#error-codes-and-the-name-field
//META: doc_section_digest=58de6e7299bd58eba9a686426fc010a16e2745f7e3231042be33437e83834630
//META: purpose=Warnings carry integer ids exactly like errors; comma-operator-used-in-expression renders as 41024.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc claims every diagnostic (errors and warnings alike) has a
// unique integer id. Verify by pinning the rendered code for the
// comma-operator warning to 41024 (decimal id appears in the
// rendered text with a single-character severity prefix).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
int main()
{
    int x = (1, 2);
//CHECK:      ^ 41024
//CHECK:      ^ comma operator used in expression
// CHECK: W41024
    return x;
}
