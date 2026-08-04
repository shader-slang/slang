//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic W32006 (enum-case-implicit-tag-value-overflow) - implicit enum case value overflows underlying tag type
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=32006
//META: catalog_name=enum-case-implicit-tag-value-overflow
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Setting an explicit case to the max of int and letting the next
// implicit case overflow triggers W32006.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

enum E
{
    A = 2147483647,
    B,
}

// CHECK: W32006
