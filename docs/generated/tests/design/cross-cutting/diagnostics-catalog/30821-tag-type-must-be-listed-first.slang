//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=316c0d656c74aed812f7ab13e6a3880b1c257fc64d3d25a4ffabd7a84268884b
//META: purpose=Fires diagnostic E30821 (tag-type-must-be-listed-first) -- an enum type may only have a single tag type, and that type must be listed first in the list of bases
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30821
//META: catalog_name=tag-type-must-be-listed-first
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

interface I {}
enum E : I, int { A }


//CHECK: E30821
