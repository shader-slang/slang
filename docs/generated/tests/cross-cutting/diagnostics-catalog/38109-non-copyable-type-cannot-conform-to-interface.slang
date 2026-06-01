//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic W38109 (non-copyable-type-cannot-conform-to-interface) - non-copyable interface conformance is not fully supported
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38109
//META: catalog_name=non-copyable-type-cannot-conform-to-interface
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A non-copyable type conforming to an interface produces W38109.

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

// CHECK: W38109
