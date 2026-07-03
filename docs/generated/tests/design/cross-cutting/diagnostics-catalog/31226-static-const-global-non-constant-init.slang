//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E31226 (static-const-global-non-constant-init) - static const global initializer must be a compile-time constant
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31226
//META: catalog_name=static-const-global-non-constant-init
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Initializing a static const global from a non-const global value
// triggers E31226.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

static float f = 1.0f;
static const int g = int(f * 25);

// CHECK: E31226
