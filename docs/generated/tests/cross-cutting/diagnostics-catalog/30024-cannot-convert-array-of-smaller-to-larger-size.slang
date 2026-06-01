//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30024 (cannot-convert-array-of-smaller-to-larger-size) - array size mismatch prevents conversion
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30024
//META: catalog_name=cannot-convert-array-of-smaller-to-larger-size
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Assigning a smaller fixed-size array to a larger fixed-size array fires the diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    int big[5];
    int small[3] = { 1, 2, 3 };
    big = small;
}

// CHECK: 30024
