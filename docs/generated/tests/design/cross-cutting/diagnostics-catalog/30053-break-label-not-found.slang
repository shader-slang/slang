//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=861e0a6b312b6b628a86e95a56e5166390488fa0565ca99c592b3c4fb3d1a441
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

//CHECK: E30053
