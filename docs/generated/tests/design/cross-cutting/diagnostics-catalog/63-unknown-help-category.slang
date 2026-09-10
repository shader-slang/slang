//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=4ea3e48540865ad5ef64e3bc3a6b19c1bd1e913b999cc2f3b710cd3eb7069711
//META: purpose=Fires diagnostic E00063 (unknown-help-category) - unknown help category
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=63
//META: catalog_name=unknown-help-category
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `-h <category>` only accepts the driver's known help categories. An unknown
// category is diagnosed rather than silently printing general help; the CHECK
// pins that code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-h bogusHelpCategory
[numthreads(1,1,1)]
void main() {}

//CHECK: E00063
