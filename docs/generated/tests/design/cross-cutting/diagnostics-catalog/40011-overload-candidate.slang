//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0d38710803487481ded085ca5cee10f82c37a05e5dea2cdbfe425963306c60a7
//META: purpose=Fires diagnostic 40011 (overload-candidate) - candidate note after overload error
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=40011
//META: catalog_name=overload-candidate
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A wrong-argument-count call produces an error followed by a
// 40011 candidate note pointing at the function signature.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void f(int a, int b) {}

void main()
{
    f(1);
}

//CHECK: 40011
