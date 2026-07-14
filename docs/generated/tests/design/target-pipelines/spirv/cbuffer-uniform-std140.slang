//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=adbc9f62b2b56295371ba0ea74f7f7385925be80ae5d4b3005d64a60b8337c56
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
