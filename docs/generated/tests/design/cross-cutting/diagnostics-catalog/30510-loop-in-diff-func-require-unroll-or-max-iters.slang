//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=b8f904aecf9e728d9c4d1931e51614ef4449f41fd9569c18771a86d6e39c87f9
//META: purpose=Fires diagnostic E30510 (loop-in-diff-func-require-unroll-or-max-iters) -- loop in differentiable function needs attributes
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30510
//META: catalog_name=loop-in-diff-func-require-unroll-or-max-iters
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[Differentiable]
float f(float x) {
    float y = x;
    for (uint i = 0; i < uint(x); ++i) { y = y * x; }
    return y;
}
[shader("compute")][numthreads(1,1,1)]
void main() {
    var p = diffPair(0.5, 1.0);
    var dx = fwd_diff(f)(p);
}


//CHECK: E30510
