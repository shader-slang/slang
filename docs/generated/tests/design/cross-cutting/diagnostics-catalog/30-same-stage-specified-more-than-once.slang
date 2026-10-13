//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=d9b2a014454276fbf20593fb1ddf17da677d7d329709ddb535ee625567df5ead
//META: purpose=Fires diagnostic E00030 (same-stage-specified-more-than-once) - the stage '~stage' was specified more than once for entry point '~entryPoint'
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=30
//META: catalog_name=same-stage-specified-more-than-once
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Repeating `-stage` for one entry point is redundant rather than fatal. The
// second identical `-stage compute` makes the driver warn; the CHECK pins the
// duplicate-stage warning code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute -stage compute
[numthreads(1,1,1)]
void main() {}

//CHECK: E00030
