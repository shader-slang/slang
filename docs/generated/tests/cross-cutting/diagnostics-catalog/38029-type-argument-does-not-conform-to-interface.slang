//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E38029 (type-argument-does-not-conform-to-interface) - type argument doesn't conform to interface
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38029
//META: catalog_name=type-argument-does-not-conform-to-interface
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A struct without the required interface is passed as a
// generic type argument.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IFoo { int f(); }
struct S {}

void g<T : IFoo>(T v) {}

void main()
{
    S s;
    g<S>(s);
}

// CHECK: E38029
