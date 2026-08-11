//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=acc46851260c03b538b73999450f55f6bb38f319109ef68c03df11675610d1f9
//META: purpose=Fires diagnostic E41031 (loss-of-derivative-assigning-to-non-differentiable-location) - derivative is lost during assignment
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=41031
//META: catalog_name=loss-of-derivative-assigning-to-non-differentiable-location
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// In a [BackwardDifferentiable] function, assigning a differentiable
// value into a non-differentiable RWStructuredBuffer location loses
// the derivative — error fires.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

RWStructuredBuffer<float> o;

[BackwardDifferentiable]
float foo(float x) { o[0] = x; return x * x; }

[numthreads(1,1,1)]
void main() {
    var p = diffPair(1.0, 1.0);
    bwd_diff(foo)(p, 1.0);
}

//CHECK: E41031
