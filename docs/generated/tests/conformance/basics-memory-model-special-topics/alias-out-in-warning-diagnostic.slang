//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T10:01:20+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-special-topics.md#function-parameters-with-inout-inout-modifiers
//META: doc_section_digest=50b3040fe642ee135f57002e37f20cd9b949f33677eec525295c406b3b231822
//META: purpose=Passing the same variable as both an `in` parameter and an `out` parameter is undefined behavior and elicits W30051 when at least one parameter is `out`/`inout`.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that passing the same variable to an `in` parameter and an `out` parameter
// also triggers W30051, not only the inout+inout case.
// The call passes `x` as both arguments to `combine(in int a, out int b)`; CHECK
// asserts the alias warning fires with the caret pointing at the `out` argument.

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
