//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=1f2e8534b4863bac46d6e49ff47196f47e4d400a446ae27b1bfef90079781dca
//META: purpose=Fires diagnostic E30079 (cannot-take-constant-pointers) - cannot take address of immutable object
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30079
//META: catalog_name=cannot-take-constant-pointers
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Taking the address of a const local should fail.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    const int x = 5;
    int* p = &x;
}

//CHECK: E30079
