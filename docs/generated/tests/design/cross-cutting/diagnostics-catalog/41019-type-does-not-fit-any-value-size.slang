//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=b8c612e0aaa6c2174553325244226310cf21d4da75bfc397a72ce2605a26681a
//META: purpose=Fires diagnostic E41019 (type-does-not-fit-any-value-size) - type does not fit in size required by interface
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=41019
//META: catalog_name=type-does-not-fit-any-value-size
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A struct that conforms to an `[anyValueSize(N)]` interface but
// exceeds the declared size limit triggers the size-fit diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -conformance "Big:IFoo=0"

[anyValueSize(8)]
interface IFoo { int get(); }

struct Big : IFoo {
    uint a, b, c;
    int get() { return int(a); }
}

//CHECK: E41019
