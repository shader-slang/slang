//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T10:01:20+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-special-topics.md#function-parameters-with-inout-inout-modifiers
//META: doc_section_digest=50b3040fe642ee135f57002e37f20cd9b949f33677eec525295c406b3b231822
//META: purpose=`in` is the default parameter direction: a parameter declared without a direction modifier emits identically to one declared with explicit `in`, both without `inout`/`out` on the HLSL signature.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: `in` is the default direction.
// In HLSL emission, a plain parameter (no direction) and a parameter
// explicitly marked `in` both emit without `inout` or `out` keywords.
// We verify both functions emit equivalent value-parameter signatures.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<int> output;

[noinline]
void readImplicit(int val)
{
    output[2] = val;
}

[noinline]
void readExplicit(in int val)
{
    output[3] = val;
}

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int x = output[0];
    readImplicit(x);
    readExplicit(x);
}

// CHECK: void readImplicit_{{[0-9]+}}(int val_{{[0-9]+}})
// CHECK-NOT: inout
// CHECK-NOT: out int
// CHECK: void readExplicit_{{[0-9]+}}(int val_{{[0-9]+}})
