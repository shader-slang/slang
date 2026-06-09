//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T10:01:20+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-special-topics.md#function-parameters-with-inout-inout-modifiers
//META: doc_section_digest=50b3040fe642ee135f57002e37f20cd9b949f33677eec525295c406b3b231822
//META: purpose=`in` parameter: the function may read it any number of times; HLSL emits a plain value parameter that is read multiple times in the function body without restriction.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an `in` parameter may be read multiple times inside the callee.
// `sumOfReads` reads `val` three times to compute a sum; CHECK asserts the HLSL
// output contains a plain value parameter signature and that `val` appears more
// than once in an addition expression, proving multiple reads are preserved.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<int> output;

[noinline]
void sumOfReads(in int val)
{
    // read `val` three times
    int r = val + val + val;
    output[0] = r;
}

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int x = output[1];
    sumOfReads(x);
}

// CHECK: void sumOfReads_{{[0-9]+}}(int val_{{[0-9]+}})
// CHECK: val_{{[0-9]+}} + val_{{[0-9]+}}
