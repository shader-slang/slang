//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=006f0d2a520f68f078a947cc768c7dd5b7191611922e1aa0cbae4612da4d11c4
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


//CHECK: E30822
