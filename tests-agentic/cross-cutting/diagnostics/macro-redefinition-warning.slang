//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T12:30:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#anatomy-of-a-legacy-diagnostic
//META: doc_section_digest=04e6b41a5ed7b7f40deee827fed8dac1f618dc77bdfc81ebd67f98a5810bb953
//META: purpose=A legacy warning with a note renders both the warning's primary message and the companion note pointing at the previous definition.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// macro-redefinition (15400) is a legacy `warning(...)` entry with a
// companion `note(...)` pointing at the original. Verify the warning
// message appears at the new definition and the note appears at the
// original definition.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#define MY_MACRO 1
/*CHECK:
        ^^^^^^^^ see previous definition of 'MY_MACRO'
*/

#define MY_MACRO 2
/*CHECK:
        ^^^^^^^^ macro redefined
        ^^^^^^^^ redefinition of macro 'MY_MACRO'
*/

void main() {}
