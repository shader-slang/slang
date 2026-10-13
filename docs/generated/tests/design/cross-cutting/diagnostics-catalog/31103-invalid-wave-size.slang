//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=55084ad1a7ccf10fbbb5e8acb33c0c1f685e109b977b7067b2bcc95d423d7e97
//META: purpose=Fires diagnostic E31103 (invalid-wave-size) - invalid 'WaveSize' value
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31103
//META: catalog_name=invalid-wave-size
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `[WaveSize]` only accepts hardware wave widths, which are powers of two. 7 is
// outside that set, and the CHECK pins the invalid-wave-size error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute
[numthreads(1,1,1)]
[WaveSize(7)]
void main() {}

//CHECK: E31103
