//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#legalizeuniformbufferload
//META: doc_section_digest=dacbe0c33c670a59f3b8ad61a7d49e89ea75e095f3ed8e7f44d44926ea0cbb13
//META: purpose=A constant buffer whose element type is not a struct keeps its whole-object load, so its uses read the cbuffer variable directly instead of a recombined brace-list.
//META: intent=boundary
//META: pipeline_stage=legalize
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The absent-form half of the `legalizeUniformBufferLoad` pair. The pass only
// rewrites loads whose pointer operand has a `ConstantBuffer` type with a
// *struct* element type; `ConstantBuffer<float4>` is left alone.
//
// Only the pair of observations distinguishes a conditional rewrite from an
// unconditional one: `uniform-buffer-load-splits-per-field.slang` shows the
// struct case splitting into a brace-list, and this test shows the non-struct
// case producing no such recombination — the use reads the cbuffer variable
// directly.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

ConstantBuffer<float4> gCB;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    float4 v = gCB;
    outBuf[0] = v.x + v.w;
}

// The cbuffer's contents are reached directly; no `makeStruct` brace-list is
// materialised for the non-struct element type. The CHECK-NOT is bounded by
// the function body, which is where the recombination would appear if the
// pass had fired.
// CHECK: cbuffer
// CHECK: void main
// CHECK-NOT: = {
// CHECK: return;
