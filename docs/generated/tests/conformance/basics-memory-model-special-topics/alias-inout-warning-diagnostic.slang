//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T10:01:20+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-special-topics.md#function-parameters-with-inout-inout-modifiers
//META: doc_section_digest=50b3040fe642ee135f57002e37f20cd9b949f33677eec525295c406b3b231822
//META: purpose=Passing the same variable to two `inout` parameters of one function call elicits diagnostic W30051 (potentially aliased argument to out/inout/ref parameter).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: it is undefined behavior to pass the same variable as an
// argument to two parameters of a function if at least one parameter is
// out/inout. The compiler diagnoses this case with W30051.
// Example taken verbatim from the doc (Example 1).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<int> output;

void addConsumeTwoValues(
    inout int i1, inout int i2, inout int o)
{
    o += i1;
    i1 = -1;
    o += i2;
    i2 = -1;
}

[numthreads(1,1,1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    int a = dispatchThreadID.x;
    int b = dispatchThreadID.y;
    // undefined behavior: a is passed twice to inout parameters
    addConsumeTwoValues(a, b, a);
//CHECK:                ^ potentially aliased argument to 'out'/'inout'/'ref' parameter
//CHECK:                ^ argument for 'inout' parameter 'i1' may alias argument for 'inout' parameter 'o'; passing the same value to both may produce unexpected results
    output[0] = a;
}
