//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T12:30:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#anatomy-of-a-legacy-diagnostic
//META: doc_section_digest=04e6b41a5ed7b7f40deee827fed8dac1f618dc77bdfc81ebd67f98a5810bb953
//META: purpose=A legacy diagnostic with `span` and `note` helpers renders a primary span on the offending site and a companion note on the previously-defined site.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The legacy `function-redefinition` diagnostic example in the doc
// uses `err(...)` plus a `note(...)` helper. The observable
// consequence is two co-located diagnostic records: the primary at
// the redeclaration site and the note at the original definition.
// Verify both source locations carry the right message text.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int bar(){ return 1; }
/*CHECK:
    ^^^ see previous definition of 'bar'
*/

int bar(){ return 2; }
/*CHECK:
    ^^^ function 'bar' already has a body
    ^^^ redeclared here
*/
