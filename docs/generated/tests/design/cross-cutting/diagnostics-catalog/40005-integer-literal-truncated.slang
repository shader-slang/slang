//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic W40005 (integer-literal-truncated) - integer literal truncated
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=40005
//META: catalog_name=integer-literal-truncated
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An integer literal that doesn't fit in the target type's
// representable range is truncated with W40005.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

static const int8_t g = 300;

// CHECK: W40005
