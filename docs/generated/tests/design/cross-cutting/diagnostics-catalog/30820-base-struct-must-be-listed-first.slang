//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=d76a7943fe5ac0797c754e05f3cde350bae02007271632579f6cef22b019a762
//META: purpose=Fires diagnostic E30820 (base-struct-must-be-listed-first) -- a struct type may only inherit from one other struct type, and that type must appear first in the list of bases
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30820
//META: catalog_name=base-struct-must-be-listed-first
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface I {}
struct A {}
struct B {}
struct C : I, A, B {}


// CHECK: E30820
