//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=f15f833400e70f33bd4f151e90be63f4887ec974cd69a44d2663e7a0d2ca3bdb
//META: purpose=Verifies that a float literal outside the range of its suffix type is reported as a diagnostic (E40009) instead of being silently folded.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A floating-point literal keeps the value as written rather than being
// clamped to the suffix type's range, so an out-of-range magnitude has to
// surface as a diagnostic. `1e50f` is far outside binary32 range; the CHECK
// pins the warning code and its caret span over the literal token.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry notMain -stage compute

void notMain()
{
    float f = 1e50f;
//CHECK:      ^^^^^ floating-point literal unrepresentable
//CHECK:      ^^^^^ float literal '1e50f' unrepresentable, converted to 'inf'
    printf("f=%f\n", f);
}
