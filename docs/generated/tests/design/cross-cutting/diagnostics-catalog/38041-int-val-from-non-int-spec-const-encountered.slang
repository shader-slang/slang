//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E38041 (int-val-from-non-int-spec-const-encountered) - cannot cast non-integer specialization constant to integer
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38041
//META: catalog_name=int-val-from-non-int-spec-const-encountered
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Using a float specialization constant as an array dimension
// requires int-folding the non-int spec const, triggering E38041.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -emit-spirv-directly -stage compute -entry main

[SpecializationConstant]
const float f = 4.0;

RWStructuredBuffer<float> outBuf;

[numthreads(1,1,1)]
void main()
{
    float arr[(int)f];
    outBuf[0] = arr[0];
}

// CHECK: E38041
