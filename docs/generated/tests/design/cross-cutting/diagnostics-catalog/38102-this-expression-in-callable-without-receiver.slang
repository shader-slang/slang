//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:41:27Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=a6fdba45662cfa7f1231541001e681233daf662137625fc2af7d09d3e85ae082
//META: purpose=Fires diagnostic E38102 (this-expression-in-callable-without-receiver) - 'this' used in callable without an instance receiver
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38102
//META: catalog_name=this-expression-in-callable-without-receiver
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -no-codegen

struct Value
{
    int member;

    static int test()
    {
        return this.member;
    }
}

//CHECK: E38102
