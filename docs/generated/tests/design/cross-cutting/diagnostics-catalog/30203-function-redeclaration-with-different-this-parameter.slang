//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:41:27Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=620bef268514d4c70174df457bd8358660f84ae5980af4bea95fc46e8daa65c2
//META: purpose=Fires diagnostic E30203 (function-redeclaration-with-different-this-parameter) - function receiver mismatch
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30203
//META: catalog_name=function-redeclaration-with-different-this-parameter
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK,non-exhaustive): -lang hlsl docs/generated/tests/design/cross-cutting/diagnostics-catalog/30203-function-redeclaration-with-different-this-parameter.slang -no-codegen

struct Value
{
    void method();
    void method() const;
};

//CHECK: E30203
