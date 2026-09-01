//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=97a2efd98698c2ef8443a82055bd3e1bc126d5e989206c4dd633dff802ebad7d
//META: purpose=Fires diagnostic E30304 (is-as-on-unrelated-concrete-types) - 'is'/'as' on unrelated concrete types
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30304
//META: catalog_name=is-as-on-unrelated-concrete-types
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `is` on two unrelated concrete types never succeeds.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct A {}
struct B {}

void main()
{
    A a;
    bool r = a is B;
}

//CHECK: 30304
