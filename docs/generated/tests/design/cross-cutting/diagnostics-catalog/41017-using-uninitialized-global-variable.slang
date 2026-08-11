//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=6776e11de638191cb13e8f055d385357ab3b24276a8462e574863bec2eda15b9
//META: purpose=Fires diagnostic E41017 (using-uninitialized-global-variable) - use of uninitialized global variable
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=41017
//META: catalog_name=using-uninitialized-global-variable
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `static` module-scope variable that is read without initialization
// triggers the uninitialized-global-variable warning.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

static int gz;
RWStructuredBuffer<int> o;

[numthreads(1,1,1)]
void main() { o[0] = gz; }

//CHECK: E41017
