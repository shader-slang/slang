//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=2de83c81f88e7ac572129a338cd2345bbcb701ae3f8deec4690d0fbfa81eefdc
//META: purpose=Fires diagnostic E38014 (entry-point-cannot-be-generic) - generic entry point used without specialization arguments
//META: intent=negative
//META: pipeline_stage=link
//META: catalog_code=38014
//META: catalog_name=entry-point-cannot-be-generic
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A generic entry point must be specialized before it can be compiled to a
// kernel. Naming an unspecialized generic with `-entry` fires the
// cannot-be-generic error, which the CHECK pins.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute
[numthreads(1,1,1)]
void main<T>() {}

//CHECK: E38014
