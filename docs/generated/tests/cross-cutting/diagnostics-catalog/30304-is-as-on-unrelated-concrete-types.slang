//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
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

// CHECK: 30304
