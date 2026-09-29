//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=035cad8fbaffd5cba139ac114c445645195fc2b11a277986926dbcf545f723db
//META: purpose=Fires diagnostic E45001 (unresolved-symbol) - unresolved external symbol
//META: intent=negative
//META: pipeline_stage=link
//META: catalog_code=45001
//META: catalog_name=unresolved-symbol
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An `extern` function declared but never defined produces an
// unresolved-external-symbol error at link time.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

extern void doSomething();

[numthreads(1,1,1)]
void main() {
    doSomething();
}

//CHECK: E45001
