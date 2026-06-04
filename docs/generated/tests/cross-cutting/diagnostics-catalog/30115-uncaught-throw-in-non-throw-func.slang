//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30115 (uncaught-throw-in-non-throw-func) - uncaught throw in non-throwing function
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30115
//META: catalog_name=uncaught-throw-in-non-throw-func
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Function not declared as throwing cannot contain an uncaught throw.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    throw 1;
}

// CHECK: E30115
