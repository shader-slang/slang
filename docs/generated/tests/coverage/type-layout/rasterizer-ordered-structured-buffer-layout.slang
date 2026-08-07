//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies a `RasterizerOrderedStructuredBuffer` is laid out through the same MutableStructuredBuffer arm as `RWStructuredBuffer`, taking a `u` register on HLSL and a StorageBuffer binding on SPIR-V.
//META: intent=characterization
//META: covers=source/slang/slang-type-layout.cpp
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `_createTypeLayout` has a dedicated `HLSLRasterizerOrderedStructuredBufferType`
// arm that is never reached by the shipped suites, even though the neighbouring
// `RWStructuredBuffer` arm is one of the hottest paths in the file. The arm
// classifies the type as `ShaderParameterKind::MutableStructuredBuffer`, i.e.
// identical to `RWStructuredBuffer`. The test declares the ROV buffer *first*
// so that, if it were classified as a read-only resource instead, it would land
// on `t0` rather than `u0`; the CHECKs pin the `u`/UAV classification on HLSL
// and the equivalent StorageBuffer binding 0 on SPIR-V.
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute

RasterizerOrderedStructuredBuffer<float> ros;
RWStructuredBuffer<float> outp;
[numthreads(1, 1, 1)]
void main()
{
    outp[0] = ros[0];
}

// The ROV buffer consumes the UAV register that a read-only classification
// would have left untouched, and `outp` is pushed to u1 behind it.
//HLSL-DAG: RasterizerOrderedStructuredBuffer<float {{.*}}ros{{[A-Za-z0-9_]*}} : register(u0)
//HLSL-DAG: RWStructuredBuffer<float {{.*}}outp{{[A-Za-z0-9_]*}} : register(u1)

//SPV-DAG: OpDecorate %ros Binding 0
//SPV-DAG: OpDecorate %ros DescriptorSet 0
//SPV-DAG: OpDecorate %outp Binding 1
