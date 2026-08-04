//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E38203 (vector-with-disallowed-element-type-encountered) - disallowed vector element type
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=38203
//META: catalog_name=vector-with-disallowed-element-type-encountered
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A vector with a vector element type is disallowed at IR
// validation time.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -emit-spirv-directly

RWStructuredBuffer<float> outputBuffer;

[numthreads(1,1,1)]
void main(uint v : SV_DispatchThreadID)
{
    vector<vector<float, 3>, 2> nested = { { 1, 2, 3 }, { 4, 5, v } };
    outputBuffer[0] = nested[0].x;
}

// CHECK: E38203
