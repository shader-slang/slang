//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:55+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/statements.md#switch-statement
//META: doc_section_digest=a46f7b8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4e5f6
//META: purpose=A case expression that does not evaluate to a compile-time constant integer is rejected (E39999).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a case label with a non-constant expression is a compile error.
// The doc states: "The expression must evaluate to a compile-time constant integer."
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int v = 1;
    int x = 0;
    int y = 2;
    switch (v)
    {
    case y:
         /*CHECK:
         ^ E39999
         */
        x = 1;
        break;
    }
}
