//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=719124392380f70c62a7eaec5d0be085dfe259bd4bd8b5283c9602a901f256ee
//META: purpose=Generic functions are specialized at the call site; the constraint solver infers the type argument from the actual argument.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says the checker solves type/value/witness constraints
// to specialize generics. Verify the user-visible consequence:
// calling `id` with an int returns an int.

//TEST:INTERPRET(filecheck=CHECK):

T id<T>(T x) { return x; }

void main()
{
    // CHECK: id_int=5
    printf("id_int=%d\n", id(5));
}
