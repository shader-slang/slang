//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=a673c0a54d2e8994c89d15ae1a8d007702b98e181529c03011f7b3bddeb3b3c5
//META: purpose=Fires diagnostic E36107 (entry-point-uses-unavailable-capability) - unavailable features in entry point
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=36107
//META: catalog_name=entry-point-uses-unavailable-capability
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An entry point may only use features its target supports. `printf` is outside
// the WGSL compute capability set, so the capability check rejects the entry
// point before emit and the CHECK pins that error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target wgsl -entry main -stage compute
[numthreads(1,1,1)]
void main() { printf("x"); }

//CHECK: E36107
