//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:41:27Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=91e4819f4525f0922ac2d4b443850ff5dcf2875abbf3b1ca9a0aec509fffcb45
//META: purpose=Fires diagnostic E30449 (abstract-storage-cannot-be-written) - property or subscript cannot be written
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30449
//META: catalog_name=abstract-storage-cannot-be-written
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -no-codegen

struct Storage
{
    property int value
    {
        get { return 0; }
    }
}

void test(Storage storage)
{
    storage.value = 1;
}

//CHECK: E30449
