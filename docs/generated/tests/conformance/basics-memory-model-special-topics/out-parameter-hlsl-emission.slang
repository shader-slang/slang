//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T12:00:00+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-special-topics.md#function-parameters-with-inout-inout-modifiers
//META: doc_section_digest=50b3040fe642ee135f57002e37f20cd9b949f33677eec525295c406b3b231822
//META: purpose=`out` parameter: HLSL emission preserves the `out` keyword on the parameter; the callee performs at least one assignment to the output variable.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an `out` parameter emits with the `out` keyword in HLSL output.
// `writeOut` assigns the result once; CHECK asserts the HLSL signature carries
// `out` and that at least one assignment to the result parameter appears.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<int> output;

[noinline]
void writeOut(out int result, int factor)
{
    result = factor * 2;
}

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int r = 0;
    int f = output[0];
    writeOut(r, f);
    output[0] = r;
}

// CHECK: void writeOut_{{[0-9]+}}(out int result_{{[0-9]+}},
// CHECK: result_{{[0-9]+}} =
