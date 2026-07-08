//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:28:39Z
//META: source_commit=7a45a039cb7fb0a34e9184c7069a96bccdf08115
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#generics-ambiguity
//META: doc_section_digest=2adc733a81916b42380544b008460df5b366206d39b999775293a73fe57f1756
//META: purpose=The reversed pack-count spelling `IntExpr == countof(Pack)` is rejected with a dedicated diagnostic (E30437) instead of being misread as a type constraint.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented one-sidedness of the pack-count constraint:
// writing the integer on the left is detected and rejected with its
// own diagnostic rather than silently parsed as an unrelated type
// constraint. CHECK pins the diagnostic code E30437.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void reversed<let N : int, each T>() where N == countof(T)
//CHECK: E30437
//CHECK: pack-count constraint must be written with `countof(...)` on the left side
//CHECK: write this constraint as `countof(Pack) == IntExpr`
{
}
