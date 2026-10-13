//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:41:27Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=353da8af8c4faefcd95d2eab56f8038eac79b0991f0628cfad6f934ec3021e64
//META: purpose=Fires diagnostic E20105 (cpp-style-trailing-this-param-const-only-on-function) - trailing `const` is not valid on this declaration
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=20105
//META: catalog_name=cpp-style-trailing-this-param-const-only-on-function
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK,non-exhaustive): -lang hlsl docs/generated/tests/design/cross-cutting/diagnostics-catalog/20105-cpp-style-trailing-this-param-const-only-on-function.slang -no-codegen

struct Value
{
    __init() const {}
}

//CHECK: E20105
