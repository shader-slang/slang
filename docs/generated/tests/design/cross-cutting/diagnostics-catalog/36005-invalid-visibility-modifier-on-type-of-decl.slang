//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=bd9e38e5c81146f8257a80bf0b3ea39dc2f3d74bc1ea492095b5240d2440fc55
//META: purpose=Fires diagnostic E36005 (invalid-visibility-modifier-on-type-of-decl) - visibility modifier not allowed
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=36005
//META: catalog_name=invalid-visibility-modifier-on-type-of-decl
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Applying `internal` to a namespace declaration triggers E36005.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

internal namespace N { int x; }

//CHECK: E36005
