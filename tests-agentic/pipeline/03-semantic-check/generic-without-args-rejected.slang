//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=719124392380f70c62a7eaec5d0be085dfe259bd4bd8b5283c9602a901f256ee
//META: purpose=A generic type used without specifying its arguments is rejected with a diagnostic explaining a generic argument is required.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says generic specialization is part of semantic check.
// Verify the user-visible consequence when a generic is referenced
// without arguments.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Box<T> { T x; }

void main()
{
    Box b;
/*CHECK:
    ^^^ generic type 'Box' used without argument
*/
// CHECK: E30408
}
