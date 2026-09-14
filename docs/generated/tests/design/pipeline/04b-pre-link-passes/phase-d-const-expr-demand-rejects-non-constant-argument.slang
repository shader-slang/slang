//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#phase-d-non-essential-validation-stripping-and-finalization
//META: doc_section_digest=de3af7e62fdf97582267b3c674b87776ce5d49cf79a3d54ae3a770a3c679cad0
//META: purpose=Phase D's propagateConstExpr (D3) diagnoses where a constexpr demand cannot be met, rejecting a runtime value passed to a constexpr parameter.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the one place D3 is user-visible. The pass is mostly a transform
// that retypes proven-constant values, and the doc says it only diagnoses
// when a constexpr demand cannot be satisfied. This is the minimal shape of
// that failure: a `uniform` global, whose value is not known until run time,
// is passed to a parameter declared `constexpr`. The caret pins the error to
// the argument and the annotated text names the parameter position.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

uniform int dyn;
uniform RWStructuredBuffer<int> buf;

int doubled(constexpr int n)
{
    return n * 2;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = doubled(dyn);
//CHECK:            ^ argument is not a compile-time constant
//CHECK:            ^ arg 1 in 'doubled' is not a compile-time constant
// CHECK: E40013
}
