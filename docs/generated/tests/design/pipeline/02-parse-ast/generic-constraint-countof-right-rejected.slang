//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#generics-ambiguity
//META: doc_section_digest=eb0fd6263e6c05fcda56c26f5dbe0c80c738c9165b8bfd51abc3dc4235097f39
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
//CHECK:                                        ^^^^^^^ error E30437
//CHECK:                                        ^^^^^^^ write this constraint as `countof(Pack) == IntExpr`
{
}
