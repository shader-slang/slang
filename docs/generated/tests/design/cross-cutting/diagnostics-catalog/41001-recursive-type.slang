//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0f8ecee8117d41eb5c4adbb1ba6de1b8af5201683232684aca4f1c3c621d4cb8
//META: purpose=Fires diagnostic E41001 (recursive-type) - type contains cyclic reference
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=41001
//META: catalog_name=recursive-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A struct that contains itself as a field has an infinite size,
// which the type-cycle detector rejects.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

struct Bad { Bad next; };

[numthreads(1,1,1)]
void main() {}

//CHECK: E41001
