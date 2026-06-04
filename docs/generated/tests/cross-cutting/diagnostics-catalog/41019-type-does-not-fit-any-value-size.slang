//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
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

// CHECK: E41019
