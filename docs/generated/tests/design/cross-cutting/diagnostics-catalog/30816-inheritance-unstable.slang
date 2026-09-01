//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=4d584151549848a93fea1f804d7f4da49922f571ff7e1327f21b75560dec1e4d
//META: purpose=Fires diagnostic E30816 (inheritance-unstable) -- support for inheritance is unstable and will be removed in future language versions, consider using composition instead
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30816
//META: catalog_name=inheritance-unstable
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct A {}
struct B : A {}


//CHECK: E30816
