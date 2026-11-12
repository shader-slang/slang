//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#anatomy-of-a-diagnostic-entry
//META: doc_section_digest=f22b494b8e869f4fe09a0366d2108166de2b12d27d47542c79c644cf98a38302
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
