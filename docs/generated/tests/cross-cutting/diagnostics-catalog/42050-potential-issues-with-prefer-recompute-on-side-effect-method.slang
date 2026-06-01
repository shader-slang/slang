//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E42050 (potential-issues-with-prefer-recompute-on-side-effect-method) - [PreferRecompute] on function with side effects
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=42050
//META: catalog_name=potential-issues-with-prefer-recompute-on-side-effect-method
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A [BackwardDifferentiable] function decorated with [PreferRecompute]
// that has side effects (writes to a buffer) triggers a warning
// because recomputation may execute the side effect twice.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

RWStructuredBuffer<float> o;

[PreferRecompute]
[BackwardDifferentiable]
float foo(no_diff float x) { o[0] = x; return x * 2.0; }

[BackwardDifferentiable]
float caller(float x) { return foo(x); }

[numthreads(1,1,1)]
void main() {
    var p = diffPair(1.0, 0.0);
    bwd_diff(caller)(p, 1.0);
}

// CHECK: E42050
