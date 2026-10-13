//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:41:27Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=13ae8b18a1a6343d9e24cfb8a2ada74751982f9fd8182c48ee70a201db8ad857
//META: purpose=Fires diagnostic E30444 (conflicting-this-param-mode-specifiers) - conflicting receiver-mode specifiers
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30444
//META: catalog_name=conflicting-this-param-mode-specifiers
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK,non-exhaustive): -lang hlsl docs/generated/tests/design/cross-cutting/diagnostics-catalog/30444-conflicting-this-param-mode-specifiers.slang -no-codegen

struct Value
{
    [mutating]
    void method() const {}
};

//CHECK: E30444
