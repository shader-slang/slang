//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:41:27Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=6934b67a9951afc33cd6c1ecbc801fb0bfd625007bac3f0fd7c990284e6dee2e
//META: purpose=Fires diagnostic E30443 (cpp-style-trailing-this-param-const-requires-hlsl) - trailing `const` is only allowed in the HLSL dialect
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30443
//META: catalog_name=cpp-style-trailing-this-param-const-requires-hlsl
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -no-codegen

struct Value
{
    void method() const {}
}

//CHECK: E30443
