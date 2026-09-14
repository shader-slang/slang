//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#phase-d-non-essential-validation-stripping-and-finalization
//META: doc_section_digest=de3af7e62fdf97582267b3c674b87776ce5d49cf79a3d54ae3a770a3c679cad0
//META: purpose=Phase D's checkAutoDiffUsages (D6) validates the bodies of [Differentiable] functions and rejects a call that a derivative cannot be propagated through.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the auto-diff body validator. A `[Differentiable]` function
// calls an ordinary function that has no derivative, which is exactly the
// shape the validator exists to catch. The code CHECK pins the error
// identity so a rewording of the message text does not silently pass.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<float> buf;

float nondiff(float x) { return x * 2.0; }

[Differentiable]
float f(float x) { return nondiff(x); }
// CHECK: E41022

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = f(1.0);
}
