//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
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

// CHECK: E41022
