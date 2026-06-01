//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E40013 (arg-is-not-constexpr) - argument is not a compile-time constant
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=40013
//META: catalog_name=arg-is-not-constexpr
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Passing a uniform (runtime) value to a constexpr-tagged parameter
// triggers the constexpr-required diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

void foo(constexpr int n) {}

[numthreads(1,1,1)]
void main(uniform int x) {
    foo(x);
}

// CHECK: E40013
