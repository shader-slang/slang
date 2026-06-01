//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30822 (base-class-must-be-listed-first) -- a class type may only inherit from one other class type, and that type must appear first in the list of bases
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30822
//META: catalog_name=base-class-must-be-listed-first
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

class A {}
interface I {}
class B : I, A {}


// CHECK: E30822
