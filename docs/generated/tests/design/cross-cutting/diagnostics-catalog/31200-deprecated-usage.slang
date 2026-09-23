//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=7a769c05627d4b7be7f34141c9466eb4af3e5c3f7e153840b6582ec307e36f7d
//META: purpose=Fires diagnostic E31200 (deprecated-usage) -- use of deprecated declaration
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31200
//META: catalog_name=deprecated-usage
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[deprecated("use newF instead")] void oldF() {}
[shader("compute")][numthreads(1,1,1)]
void main() { oldF(); }


//CHECK: E31200
