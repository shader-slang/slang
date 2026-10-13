//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:41:27Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=17ea594ddada4b3683ca84aa84d3b35283006f5a0fb2e9cbf389997c3c1825aa
//META: purpose=Fires diagnostic E30450 (abstract-storage-has-no-address) - property or subscript does not provide an address
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30450
//META: catalog_name=abstract-storage-has-no-address
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -no-codegen

struct Storage
{
    int backing;

    property int value
    {
        get { return backing; }
        set { backing = newValue; }
    }
}

Ref<int> test(Storage storage)
{
    return storage.value;
}

//CHECK: E30450
