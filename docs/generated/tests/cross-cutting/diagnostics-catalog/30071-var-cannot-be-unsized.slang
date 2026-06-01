//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30071 (var-cannot-be-unsized) - cannot instantiate unsized type
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30071
//META: catalog_name=var-cannot-be-unsized
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A local variable of an unsized array type is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    int unsized[];
}

// CHECK: 30071
