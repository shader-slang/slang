//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=66494d0d8be468bbc3209dd2158ea5cf67ec78c7f4e8f101d9d09c84ea2bebbc
//META: purpose=Fires diagnostic E00014 (unknown-profile) - unknown profile '~profile'
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=14
//META: catalog_name=unknown-profile
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `-profile` only accepts profile names the driver knows. Passing a made-up name
// makes option parsing fail; the CHECK pins the unknown-profile code so the
// rejection cannot silently degrade into a default profile.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-profile bogusprofile
[numthreads(1,1,1)]
void main() {}

//CHECK: E00014
