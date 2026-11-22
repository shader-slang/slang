//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#severity-levels
//META: doc_section_digest=3ea14a1946d125d6e51f1168520ddc4e04afd09127febe10b5079af9e30b1816
//META: purpose=A companion note renders with the user-facing severity name `note`.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// function-redefinition (E30201) carries a companion diagnostic at
// Severity::Note pointing at the original definition. Verify that
// the rendered severity word on the note line is "note", and that
// the primary diagnostic line carries "error" per getSeverityName().

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int foo(){ return 1; }
/*CHECK:
    ^^^ see previous definition of 'foo'
*/

int foo(){ return 2; }
/*CHECK:
    ^^^ function 'foo' already has a body
    ^^^ redeclared here
*/
