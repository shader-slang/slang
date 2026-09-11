//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=fcc33b32baa9afab870a8884ecaa3b06fbb26f420f6905fd21e0108c63da89f9
//META: purpose=Fires diagnostic E41018 (returning-with-uninitialized-out) - returning without initializing out parameter
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=41018
//META: catalog_name=returning-with-uninitialized-out
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A function with an `out` parameter that returns without writing to
// that parameter triggers the returning-with-uninitialized warning.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

void f(out int x) { return; }

[numthreads(1,1,1)]
void main() {
    int a;
    f(a);
}

//CHECK: E41018
