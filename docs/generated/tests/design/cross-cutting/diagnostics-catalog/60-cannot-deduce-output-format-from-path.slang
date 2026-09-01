//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=82a08704d51973726f5f53442ce873847505ba6b506d1f1c36e9f56b550f20f5
//META: purpose=Fires diagnostic E00060 (cannot-deduce-output-format-from-path) - cannot infer an output format from the output path '~path'
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=60
//META: catalog_name=cannot-deduce-output-format-from-path
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// With no `-target`, the driver infers the output format from the `-o` path's
// extension. `.xyz` maps to no known format, so the CHECK pins the
// cannot-infer-format error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-entry main -stage compute -o out.xyz
[numthreads(1,1,1)]
void main() {}

//CHECK: E00060
