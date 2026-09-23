//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=5c860109d4b1b277568160f7867d990ba583cb37de73648ea047f7798f78519f
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


//CHECK: E30820
