//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=fed1a0f5fb506d6d17a2f4abb7e63d5a4e077f8d625002f32ccbf129ff6be053
//META: purpose=Fires diagnostic E31132 (derivative-member-attribute-can-only-be-used-on-members) -- DerivativeMember on non-member
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31132
//META: catalog_name=derivative-member-attribute-can-only-be-used-on-members
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[DerivativeMember(x)]
float f;


// CHECK: E31132
