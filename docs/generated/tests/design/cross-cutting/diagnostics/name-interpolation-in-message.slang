//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#anatomy-of-a-diagnostic-entry
//META: doc_section_digest=79bb0fc1ac8583fef268d4e89fe89d0e6aa8ce66124dff7c481098ca077adbae
//META: purpose=A `~name:Name` interpolation parameter renders the supplied name, quoted, inside the span message.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `~name:Name` interpolation parameter reaches the rendered
// text. An undeclared identifier is referenced and the CHECK asserts the
// message contains that exact name in quotes, which can only happen if the
// call site's argument was substituted into the span message.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
void main()
{
    int a = mysteryIdent;
//CHECK:    ^^^^^^^^^^^^ undefined identifier
//CHECK:    ^^^^^^^^^^^^ undefined identifier 'mysteryIdent'.
}
