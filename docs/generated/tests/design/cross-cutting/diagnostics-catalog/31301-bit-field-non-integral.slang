//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=4117d434d32eb67274ca82cd291e0e31df218e6b60d566d0d5871f5f84e175e5
//META: purpose=Fires diagnostic E31301 (bit-field-non-integral) - bit-field type must be integral
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31301
//META: catalog_name=bit-field-non-integral
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Bit-fields are only defined for integral storage. Declaring one on a `float`
// member is rejected, which the CHECK observes by code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute
struct S { float a : 4; }
[numthreads(1,1,1)]
void main() { S s; }

//CHECK: E31301
