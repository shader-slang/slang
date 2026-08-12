//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0c500bd9212cb1ee1483f1128b90f164872d1312dfa8c67341ede85de7ac04f2
//META: purpose=Fires diagnostic E30831 (cannot-inherit-from-implicitly-sealed-declaration-in-another-module) -- cannot inherit from type '~baseType:Type' in module '~moduleName:Name' because it is implicitly 'sealed'
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30831
//META: catalog_name=cannot-inherit-from-implicitly-sealed-declaration-in-another-module
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S : String {}


//CHECK: E30831
