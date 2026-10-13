//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:41:27Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=53fe38ba2b291da3d661980e3adfa7a16ad84a37126902af11ab65237a1155ca
//META: purpose=Fires diagnostic E30448 (abstract-storage-cannot-be-read) - property or subscript cannot be read
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30448
//META: catalog_name=abstract-storage-cannot-be-read
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -no-codegen

struct Storage
{
    property int value
    {
        set {}
    }
}

int test(Storage storage)
{
    return storage.value;
}

//CHECK: E30448
