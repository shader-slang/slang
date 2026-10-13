//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:41:27Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=855321d9de4eef3d1e4d8ccc25d4fff4ea98c5f3a69dd121650bea534de966ee
//META: purpose=Fires diagnostic E30446 (lambda-parameter-type-depends-on-runtime-existential) - lambda parameter type depends on a runtime existential
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30446
//META: catalog_name=lambda-parameter-type-depends-on-runtime-existential
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -no-codegen

interface IValue
{
    associatedtype Value;
}

void test(IValue value)
{
    let function = (value.Value element) => element;
}

//CHECK: E30446
