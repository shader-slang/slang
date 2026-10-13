//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:41:27Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=e3664b6d5d92c0a248336c66647537341cbdd47cb5d7051b8030018617972ff6
//META: purpose=Fires diagnostic E30447 (lambda-capture-type-depends-on-runtime-existential) - lambda capture type depends on a runtime existential
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30447
//META: catalog_name=lambda-capture-type-depends-on-runtime-existential
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -no-codegen

interface IValue
{
    int getValue();
}

interface IProducer
{
    associatedtype Result : IValue;
    Result getResult();
}

void test(IProducer producer)
{
    let result = producer.getResult();
    let function = () => result.getValue();
}

//CHECK: E30447
