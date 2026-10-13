//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:41:27Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=e1e78668d8abe195a03013dc5a95617e8688e562cd3bcf42cb59af9f80665535
//META: purpose=Fires diagnostic E40026 (ref-accessor-return-must-be-l-value) - a `ref` accessor must return an l-value
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=40026
//META: catalog_name=ref-accessor-return-must-be-l-value
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -no-codegen

struct Storage
{
    property int value
    {
        ref
        {
            return 1;
        }
    }
}

//CHECK: E40026
