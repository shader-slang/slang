//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:41:27Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=16fc9675bc53057b13852a7bc28cab203313aaaba67a7437c1b53c574804d49e
//META: purpose=Fires diagnostic E30445 (lambda-return-type-depends-on-runtime-existential) - lambda return type depends on a runtime existential
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30445
//META: catalog_name=lambda-return-type-depends-on-runtime-existential
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -no-codegen

interface IValue
{
    associatedtype Value;
    Value getValue();
}

void test(IValue value)
{
    let function = () => value.getValue();
}

//CHECK: E30445
