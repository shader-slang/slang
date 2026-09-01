//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=bf9f31f6f07ab1a177c9cffd63368d298c891107671577467228a7dd01ab994e
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

//CHECK: E38029
