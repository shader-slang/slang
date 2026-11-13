//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=8c569fc3a505737867c26481a28d0bae31f23172e453102fec283d0fdcde62bf
//META: purpose=The constraint solver infers a generic function's type argument from the actual argument at the call site.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the basic inference path the solver's fixpoint provides. No
// explicit generic argument is written, so `T` can only come from unifying the
// parameter type with the argument type at each call. Two different argument
// types go through the same declaration; CHECK asserts each returned its own
// value, which requires two distinct successful inferences.

//TEST:INTERPRET(filecheck=CHECK):

T identity<T>(T v)
{
    return v;
}

void main()
{
    // CHECK: i=6 f=1.5
    printf("i=%d f=%.1f\n", identity(6), identity(1.5f));
}
