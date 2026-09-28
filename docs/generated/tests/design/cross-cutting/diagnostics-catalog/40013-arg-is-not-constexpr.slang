//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=eb2ade86a2759804798dd8d94c941e4cdcf354d8b13d84064fb31963b36ec133
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

//CHECK: E40013
