//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T10:01:20+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-special-topics.md#function-parameters-with-inout-inout-modifiers
//META: doc_section_digest=50b3040fe642ee135f57002e37f20cd9b949f33677eec525295c406b3b231822
//META: purpose=`inout` parameter: HLSL emission preserves the `inout` keyword on the parameter, and multiple reads/writes in the body emit as sequential assignments to the parameter variable.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: the function may read and write the argument passed to
// the inout-parameter any number of times during the function call.
// In HLSL, Slang emits `inout` directly on the parameter, and multiple
// reads/writes in the body appear as sequential in-place assignments.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<int> output;

[noinline]
void multiReadWrite(inout int val)
{
    // multiple reads and writes of inout parameter
    int r1 = val;
    int r2 = val;
    val = r1 + r2;
    int r3 = val;
    val = r3 * 2;
}

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int x = output[0];
    multiReadWrite(x);
    output[0] = x;
}

// CHECK: void multiReadWrite_{{[0-9]+}}(inout int val_{{[0-9]+}})
// CHECK: val_{{[0-9]+}} =
