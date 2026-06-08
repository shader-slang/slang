//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E30100 (static-ref-to-non-static-member) - type cannot refer to non-static member
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30100
//META: catalog_name=static-ref-to-non-static-member
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Referring to a non-static struct member via the type name is invalid.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S { int x; };

void main()
{
    int v = S.x;
}

// CHECK: 30100
