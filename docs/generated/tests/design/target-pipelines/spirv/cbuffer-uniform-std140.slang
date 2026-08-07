//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0e5b4c31fb3cb504b87417d8391d1c84f2175e1be41f81660fa7556568fce3ac
//META: purpose=A cbuffer { ... } block lowers via legalizeConstantBufferLoadForGLSL to a *_std140 Block-decorated struct in Uniform storage class.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// SPIR-V uses Uniform storage for HLSL-style constant buffers
// with the std140 packing rules. The lowering wraps the cbuffer
// fields in a *_std140-suffixed Block struct.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

cbuffer Globals { float4 g; }
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = g;
}

// CHECK-DAG: OpDecorate %{{[a-zA-Z_0-9]+}}_std140 Block
// CHECK: OpVariable %_ptr_Uniform_{{[a-zA-Z_0-9]+}} Uniform
