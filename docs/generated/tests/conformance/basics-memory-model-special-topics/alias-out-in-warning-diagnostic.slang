//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T10:01:20+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-special-topics.md#function-parameters-with-inout-inout-modifiers
//META: doc_section_digest=50b3040fe642ee135f57002e37f20cd9b949f33677eec525295c406b3b231822
//META: purpose=Passing the same variable as both an `in` parameter and an `out` parameter is undefined behavior and elicits W30051; the doc says "at least one parameter is out/inout".
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: it is UB to pass the same variable to two parameters of a
// function if at least one parameter is out/inout.
// This test covers the case where one parameter is `in` and the other is `out`
// (not inout+inout as in Example 1 of the doc). W30051 must still fire.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<int> output;

void combine(in int a, out int b)
{
    b = a * 2;
}

[numthreads(1,1,1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    int x = output[0];
    // undefined behavior: x passed as both in and out
    combine(x, x);
//CHECK:       ^ potentially aliased argument to 'out'/'inout'/'ref' parameter
//CHECK:       ^ argument for 'out' parameter 'b' may alias argument for 'in' parameter 'a'; passing the same value to both may produce unexpected results
    output[0] = x;
}
