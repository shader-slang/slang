//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:41:27Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=5ff75b083c907b4ccfb953b6d5d3f16335c8a28c99d3305835751da677a36934
//META: purpose=Fires diagnostic E30451 (interface-value-has-no-address) - interface value does not provide an address
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30451
//META: catalog_name=interface-value-has-no-address
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -no-codegen

interface IValue
{
    [__ref]
    void mutate();
}

void test(IValue value)
{
    value.mutate();
}

//CHECK: E30451
