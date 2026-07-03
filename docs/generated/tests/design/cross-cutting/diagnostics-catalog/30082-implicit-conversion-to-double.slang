//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic W30082 (implicit-conversion-to-double) - implicit float-to-double conversion
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30082
//META: catalog_name=implicit-conversion-to-double
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Assigning a float to a double without explicit cast emits the warning.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    float f = 1.0;
    double d = f;
}

// CHECK: 30082
