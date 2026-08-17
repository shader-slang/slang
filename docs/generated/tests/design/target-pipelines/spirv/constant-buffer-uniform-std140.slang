//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=61b9f1fa9e25e0a1a331e30835332c142e930cea0b3de2b952f901beda9edd64
//META: purpose=ConstantBuffer<T> lowers identically to cbuffer { T fields } - a *_std140 Block struct in Uniform storage class.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// ConstantBuffer<T> is HLSL surface syntax for the same lowering
// produced by cbuffer. Both reach Phase C's
// legalizeConstantBufferLoadForGLSL and emit a *_std140 Block
// struct in Uniform storage class.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

struct Params { float4 g; }
ConstantBuffer<Params> cb;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = cb.g;
}

// CHECK-DAG: OpDecorate %Params_std140 Block
// CHECK: OpVariable %_ptr_Uniform_Params_std140 Uniform
