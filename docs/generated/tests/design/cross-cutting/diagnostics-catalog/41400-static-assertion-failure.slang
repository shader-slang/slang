//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=20479c0355317794a5f67c6546b598f3860019bdbd84959e50ea83f85b8d45fa
//META: purpose=Fires diagnostic E41400 (static-assertion-failure) - static assertion failed
//META: intent=negative
//META: pipeline_stage=emit
//META: catalog_code=41400
//META: catalog_name=static-assertion-failure
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A static_assert(false, "msg") with a literal-string message
// triggers the static-assertion-with-message failure diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

[numthreads(1,1,1)]
void main() {
    static_assert(false, "math is broken");
}

//CHECK: E41400
