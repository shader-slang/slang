//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E30101 (cannot-dereference-type) - cannot dereference type
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30101
//META: catalog_name=cannot-dereference-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Dereferencing a non-pointer scalar should fail.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    int x = 5;
    int y = *x;
}

// CHECK: E30101
