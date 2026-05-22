//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T12:30:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#severity-levels
//META: doc_section_digest=6c1c702cf80ed3f86a13f8071f7debe50ebc4cb725e9ba9101ed110bca3e30e0
//META: purpose=A diagnostic emitted at Severity::Note renders with the user-facing name "note".
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// function-redefinition (E30201) carries a companion diagnostic at
// Severity::Note pointing at the original definition. Verify that
// the rendered severity word on the note line is "note", and that
// the primary diagnostic line carries "error" per getSeverityName().

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int foo(){ return 1; }
/*CHECK:
    ^^^ note
*/

int foo(){ return 2; }
/*CHECK:
    ^^^ error
    ^^^ redeclared here
*/
//CHECK: E30201
