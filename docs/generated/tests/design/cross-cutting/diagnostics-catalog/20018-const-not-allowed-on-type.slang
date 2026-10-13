//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=cde5b8099b88ea4c51f0c9faf4d0e838ff3feacf573e80791035b991bb84afa6
//META: purpose=Fires diagnostic E20018 (const-not-allowed-on-type) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=20018
//META: catalog_name=const-not-allowed-on-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E20018.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
struct Wrap<T> { T value; }
Wrap<const int> w;
void main() { w.value; }

//CHECK: E20018
