//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=b9c754724df24a366e6be644469746acf0b782816bfb20875a5f319bf53bbedf
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

//CHECK: E30115
