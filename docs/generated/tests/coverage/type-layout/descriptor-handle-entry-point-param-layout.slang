//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies a `DescriptorHandle<T>` entry-point parameter is laid out as a varying parameter through GetDescriptorHandleLayout, taking a uint2 varying slot on SPIR-V and the native handle type on CUDA and CPU.
//META: intent=characterization
//META: covers=source/slang/slang-type-layout.cpp
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `GetDescriptorHandleLayout` is a virtual on the simple layout rules with four
// overrides (default, GLSL, CPU, CUDA), and the only caller is the
// `DescriptorHandleType` arm of `createVaryingParameterTypeLayout`. Reaching it
// therefore requires the handle to appear as an *entry-point parameter*, not as
// a global: a global `DescriptorHandle` goes down the ordinary
// `_createTypeLayout` path instead. The SPIR-V CHECK pins the default rule's
// answer — a handle is a `uint2`-sized varying input, so each parameter takes
// one `Location` — while the CUDA and CPP CHECKs pin the two target-specific
// overrides, which map the handle back onto the underlying resource's own
// layout (`CUtexObject` / `SamplerState` on CUDA, the resource types on CPU).
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

RWStructuredBuffer<float4> outp;
[shader("compute")]
[numthreads(1, 1, 1)]
void main(DescriptorHandle<Texture2D<float4> > tex, DescriptorHandle<SamplerState> samp)
{
    outp[0] = tex.SampleLevel(samp, float2(0, 0), 0);
}

// Two uint2 varying inputs, one Location each.
//SPV-DAG: OpDecorate %tex Location 0
//SPV-DAG: OpDecorate %samp Location 1
//SPV-DAG: %tex = OpVariable %_ptr_Input_v2uint Input

// CUDA collapses the pair onto the combined texture object plus a sampler.
//CUDA: __global__ void main{{[A-Za-z0-9_]*}}(CUtexObject tex{{[A-Za-z0-9_]*}}, SamplerState samp{{[A-Za-z0-9_]*}})

// CPU keeps the two resources as separate entry-point params.
//CPP-DAG: Texture2D<Vector<float, 4> > tex{{[A-Za-z0-9_]*}};
//CPP-DAG: SamplerState samp{{[A-Za-z0-9_]*}};
