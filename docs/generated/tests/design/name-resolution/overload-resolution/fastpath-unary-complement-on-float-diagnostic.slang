//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#the-builtin-operator-fast-path
//META: doc_section_digest=16e17f0dd7f2f633f3f55fe47544e0f4c04b1b5b2ce0bf083439c29eeb4a3acc
//META: purpose=Unary ~ applied to a builtin floating-point operand is diagnosed by the fast path with bitwise-operator-requires-integer-operands.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the unary arm of the fast path's own diagnosis: `~` on a float
// operand is rejected before candidate collection. CHECK pins the
// bitwise/shift operand diagnostic on the `~` token.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    float f = 1.0f;
    float r = ~f;
//CHECK:      ^ bitwise/shift operator '~' requires integer operands
    printf("%f\n", r);
}
