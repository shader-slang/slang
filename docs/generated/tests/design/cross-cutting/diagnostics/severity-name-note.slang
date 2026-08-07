//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#severity-levels
//META: doc_section_digest=45a1f9ffaa6abf3626a600faa360e7aa4723d4e67c65508783a32e11ac892cf2
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
