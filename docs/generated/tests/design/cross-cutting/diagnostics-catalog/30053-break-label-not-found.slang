//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E30053 (break-label-not-found) - break label not found
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30053
//META: catalog_name=break-label-not-found
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Using `break` with a label that no enclosing loop bears triggers the diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    for (int i = 0; i < 10; ++i)
    {
        break nonexistent_label;
    }
}

// CHECK: E30053
