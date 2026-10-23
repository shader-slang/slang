//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T10:01:20+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-special-topics.md#function-parameters-with-inout-inout-modifiers
//META: doc_section_digest=50b3040fe642ee135f57002e37f20cd9b949f33677eec525295c406b3b231822
//META: purpose=Passing the same variable as both an `in` parameter and an `out` parameter elicits no W30051, because the `in` argument is copied before the call; the warning applies only when both parameters may refer to the argument's storage.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that passing the same variable to an `in` parameter and an `out` parameter
// does not trigger W30051, unlike the inout+inout case.
// The call passes `x` as both arguments to `combine(in int a, out int b)` and carries
// no annotation: DIAGNOSTIC_TEST is exhaustive by default, so a W30051 on it fails the
// test. A control call to `combineInout`, which differs only in taking `a` as `inout`,
// does warn, which shows that the absence comes from the `in` mode.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<int> output;

void combine(in int a, out int b)
{
    b = a * 2;
}

void combineInout(inout int a, out int b)
{
    b = a * 2;
}

[numthreads(1,1,1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    int x = output[0];
    // `a` receives a copy of `x` before the call, so writing `b` cannot change `a`.
    combine(x, x);
    // Control: with `a` as `inout`, both parameters may refer to the storage of `x`.
    combineInout(x, x);
//CHECK:         ^ potentially aliased argument to a parameter that may be written
//CHECK:         ^ argument for 'inout' parameter 'a' may alias argument for 'out' parameter 'b'; passing the same value to both may produce undefined behavior
    output[0] = x;
}
