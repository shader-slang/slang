//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=1074496b28085147403fafe4e7f6ef66cdac300ce970e98ba2ff1a8d5ac3b3a9
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

//CHECK: E38041
