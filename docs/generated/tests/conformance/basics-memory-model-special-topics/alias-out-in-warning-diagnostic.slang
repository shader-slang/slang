//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-02T10:45:10+00:00
//META: source_commit=6815ed82d5e8c95b0e1258ca2eba0ca9c7825d86
//META: doc_ref=docs/language-reference/basics-memory-model-special-topics.md#function-parameters-with-inout-inout-modifiers
//META: doc_section_digest=50b3040fe642ee135f57002e37f20cd9b949f33677eec525295c406b3b231822
//META: purpose=Passing the same variable to two `out` parameters is undefined behavior and elicits W30051.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the W30051 alias warning for the writable-writable pairing:
// passing the same variable to two `out` parameters is undefined behavior
// per the cited section, and the compiler warns about it. The `in`+`out`
// pairing this file used to cover no longer warns -- the `in` argument is a
// copy taken before the call -- and the section requires the behavior to be
// undefined, not a warning, so that pairing is no longer asserted here.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<int> output;

void setBoth(out int a, out int b)
{
    a = 1;
    b = 2;
}

[numthreads(1,1,1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    int y = output[1];
    // undefined behavior: y passed to two `out` parameters
    setBoth(y, y);
//CHECK:    ^ potentially aliased argument to a parameter that may be written
//CHECK:    ^ argument for 'out' parameter 'a' may alias argument for 'out' parameter 'b'; passing the same value to both may produce undefined behavior
    output[1] = y;
}
