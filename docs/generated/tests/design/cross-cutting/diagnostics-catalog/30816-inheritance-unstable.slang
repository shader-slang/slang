//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=eab125f929759194ae6bc65cbe4fbd0da69769d7cdfeeb1a6e9106a038523aba
//META: purpose=Fires diagnostic E30816 (inheritance-unstable) -- support for inheritance is unstable and will be removed in future language versions, consider using composition instead
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30816
//META: catalog_name=inheritance-unstable
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct A {}
struct B : A {}


// CHECK: E30816
