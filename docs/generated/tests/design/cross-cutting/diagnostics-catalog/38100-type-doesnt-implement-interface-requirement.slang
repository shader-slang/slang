//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=03a7cd275c07b6c5652cd96fc4d3a953c0f3be5c5614de05aed8c8d35fc2993f
//META: purpose=Fires diagnostic E38100 (type-doesnt-implement-interface-requirement) - missing interface member
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38100
//META: catalog_name=type-doesnt-implement-interface-requirement
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A struct declared to conform to an interface but missing one
// of its required members triggers E38100.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IFoo
{
    int f();
}

struct S : IFoo
{
}

//CHECK: E38100
