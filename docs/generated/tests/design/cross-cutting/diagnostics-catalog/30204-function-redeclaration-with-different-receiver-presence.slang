//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:41:27Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=999e410b26566eb5d795b4fcaf4526095ad8c0b443dba16f7cf6f499a0ddc0c4
//META: purpose=Fires diagnostic E30204 (function-redeclaration-with-different-receiver-presence) - function receiver mismatch
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30204
//META: catalog_name=function-redeclaration-with-different-receiver-presence
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK,non-exhaustive): -lang hlsl docs/generated/tests/design/cross-cutting/diagnostics-catalog/30204-function-redeclaration-with-different-receiver-presence.slang -no-codegen

struct Value
{
    static void method();
    void method();
};

//CHECK: E30204
