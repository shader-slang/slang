//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=87c7a5cf58c986da600e24a826c8ad460ae24ebb55d959e3f5399da3bf98f654
//META: purpose=Fires diagnostic E31202 (duplicate-modifier) - duplicate modifier
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31202
//META: catalog_name=duplicate-modifier
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Repeating a modifier on one declaration is redundant. Writing `static static`
// fires the duplicate-modifier error, which the CHECK pins.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute
[numthreads(1,1,1)]
void main() { static static int x = 0; }

//CHECK: E31202
