//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=528e016ba3c5ecad33cce595d2157bd366c568370e82d1f12e2d937432a0d6b4
//META: purpose=Fires diagnostic E30830 (cannot-inherit-from-explicitly-sealed-declaration-in-another-module) -- cannot inherit from type '~baseType:Type' marked 'sealed' in module '~moduleName:Name'
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30830
//META: catalog_name=cannot-inherit-from-explicitly-sealed-declaration-in-another-module
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S : __BuiltinIntegerType {}


//CHECK: E30830
