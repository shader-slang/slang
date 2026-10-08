//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=4ce295989abe1adcd115efa7a42fe481177cac767b744965c4d81ebdf5bd900f
//META: purpose=Fires diagnostic E38109 (non-copyable-type-cannot-conform-to-interface) - non-copyable interface conformance is not fully supported
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38109
//META: catalog_name=non-copyable-type-cannot-conform-to-interface
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A non-copyable type conforming to an interface produces E38109.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IFoo
{
    int f();
}

[__NonCopyableType]
struct S : IFoo
{
    int f() { return 0; }
}

//CHECK: E38109