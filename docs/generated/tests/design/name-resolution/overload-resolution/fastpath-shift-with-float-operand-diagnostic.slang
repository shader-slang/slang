//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#the-builtin-operator-fast-path
//META: doc_section_digest=1c3af19385b1b45e53fc2911de4a5495ab14067ddf0d2925bb4569db9c7887aa
//META: purpose=A shift operator with a builtin floating-point operand is diagnosed by the fast path itself with bitwise-operator-requires-integer-operands rather than falling through to a no-overload message.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the fast path diagnoses a float shift operand directly instead
// of handing the expression to overload resolution. CHECK pins the
// bitwise/shift operand diagnostic on the `<<` token; a fall-through would
// instead report "no overload for 'operator<<'".

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    float f = 1.0f;
    int amount = 2;
    int r = f << amount;
//CHECK:      ^^ bitwise/shift operator '<<' requires integer operands
    printf("%d\n", r);
}
