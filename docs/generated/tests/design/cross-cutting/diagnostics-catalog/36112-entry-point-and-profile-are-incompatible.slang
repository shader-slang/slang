//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic W36112 (entry-point-and-profile-are-incompatible) - entry point incompatible with profile
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=36112
//META: catalog_name=entry-point-and-profile-are-incompatible
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Declaring an entry point as `[shader("compute")]` while
// requesting compilation with a vertex profile triggers W36112.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -entry main -profile vs_6_0

[shader("compute")]
[numthreads(1,1,1)]
void main() {}

// CHECK: W36112
