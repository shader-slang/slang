//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
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

// CHECK: E41031
