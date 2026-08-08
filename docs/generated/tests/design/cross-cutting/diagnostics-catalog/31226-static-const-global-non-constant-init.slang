//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=706ed89d2a2d710344758c96f66b613cd2486c33fa28290ba6f07d8b306ec176
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

//CHECK: E31226
