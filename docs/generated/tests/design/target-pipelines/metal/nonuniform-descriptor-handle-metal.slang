//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination
//META: doc_section_digest=5fdd0ac6daa767e15b8f1dad12d2d7a2846fe891f83f100e2466f794797eb465
//META: purpose=The Metal-reachable non-uniform surface is `nonuniform(handle)` on a `DescriptorHandle<T>`, which carries no capability requirement and compiles to the plain underlying resource access.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the non-uniform marker Metal can actually accept reaches
// emit without diagnostics and without leaving any residue: the descriptor
// handles resolve to an ordinary texture and sampler inside the parameter
// block, and the sample is emitted as a direct method call. The CHECK
// asserts the resolved resource types plus the sample, which is only
// reachable if the nonuniform marker was consumed rather than rejected.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

struct Params
{
    DescriptorHandle<Texture2D<float4>> tex;
    DescriptorHandle<SamplerState> samp;
};

ParameterBlock<Params> pb;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = nonuniform(pb.tex).SampleLevel(nonuniform(pb.samp), float2(0, 0), 0);
}

// CHECK-DAG: texture2d<float, access::sample> tex_
// CHECK-DAG: sampler samp_
// CHECK: .sample(
