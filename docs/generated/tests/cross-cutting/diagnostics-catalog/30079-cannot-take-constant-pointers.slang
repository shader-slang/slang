//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
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

// CHECK: E30079
