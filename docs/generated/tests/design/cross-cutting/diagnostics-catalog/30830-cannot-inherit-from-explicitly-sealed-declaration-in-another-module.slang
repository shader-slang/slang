//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=2a5fefc6f7e55273d0e7714e6813f978d35af07876e46d9fc2a091aa4d620f05
//META: purpose=Fires diagnostic E30830 (cannot-inherit-from-explicitly-sealed-declaration-in-another-module) -- cannot inherit from type '~baseType:Type' marked 'sealed' in module '~moduleName:Name'
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30830
//META: catalog_name=cannot-inherit-from-explicitly-sealed-declaration-in-another-module
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S : __BuiltinIntegerType {}


// CHECK: E30830
