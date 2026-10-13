//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=d535dcfb255d0ea357e335c298ee803762ae973821ac090d705c6b6c177fed6d
//META: purpose=Fires diagnostic E31111 (unknown-diagnostic-name) - unknown diagnostic
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=31111
//META: catalog_name=unknown-diagnostic-name
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `-warnings-disable` resolves each identifier against the diagnostic catalog.
// A name that is in no catalog entry is reported rather than ignored, so the
// CHECK pins the unknown-diagnostic-name error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute -warnings-disable not-a-real-diagnostic-name
[numthreads(1,1,1)]
void main() {}

//CHECK: E31111
