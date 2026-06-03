//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
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

// CHECK: E41001
