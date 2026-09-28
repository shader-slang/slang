//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0946846bb39448f14c9d6d0c2d7969621fd0ac33a340d3d3d73b1d7f8debd67b
//META: purpose=Fires diagnostic E41022 (loss-of-derivative-due-to-call-of-non-differentiable-function) - derivative cannot be propagated through non-differentiable call
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=41022
//META: catalog_name=loss-of-derivative-due-to-call-of-non-differentiable-function
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A [Differentiable] function calling a non-differentiable function
// without `no_diff` cannot propagate the derivative — error fires.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

float bad(float x) { return x*x; }

[Differentiable]
float foo(float x) { return bad(x); }

[numthreads(1,1,1)]
void main() {
    DifferentialPair<float> r = fwd_diff(foo)(DifferentialPair<float>(1.0, 1.0));
}

//CHECK: E41022
