//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#effect-of-shouldlegalizeexistentialandresourcetypes--false
//META: doc_section_digest=1b877aa959e4517ed3f370da15d149355d0a30955136c104ab91364155e5f28b
//META: purpose=Because shouldLegalizeExistentialAndResourceTypes is false for CUDA, legalizeResourceTypes is skipped and a resource-carrying struct keeps its texture and sampler as fields through the call, where HLSL scalarizes them out into extra function parameters.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the resource half of the flag's documented effect. CUDA
// handles resource handles as direct CUDA types, so a struct with a
// Texture2D and a SamplerState stays one aggregate and is passed by
// pointer; the doc frames this as CUDA not needing the shape
// Slang's generic legalization would produce for the other targets,
// so the sibling -target hlsl directive shows what that shape is —
// the same helper taking the resources as separate leading
// parameters. The struct is read through a ConstantBuffer so the
// fields are genuinely resource-typed rather than folded away.

//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

struct Material
{
    Texture2D<float4> albedo;
    SamplerState samp;
    float scale;
};

ConstantBuffer<Material> mat;
RWStructuredBuffer<float4> outputBuffer;

float4 shade(Material m, float2 uv)
{
    return m.albedo.SampleLevel(m.samp, uv, 0.0f) * m.scale;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = shade(mat, float2(tid.xy));
}

// On CUDA the resource stays a field of the struct...
// CUDA: struct Material_{{[0-9]+}}
// CUDA: CUtexObject albedo_{{[0-9]+}};
// ...and the callee takes the whole aggregate by pointer, with no
// resource parameters split out of it.
// CUDA: __device__ float4 {{.*}}shade_{{[0-9]+}}(Material_{{[0-9]+}} {{.*}}m_{{[0-9]+}}, float2 {{.*}}uv_{{[0-9]+}})

// On HLSL the same callee has the texture and sampler hoisted into
// their own parameters by legalizeResourceTypes.
// HLSL: float4 shade_{{[0-9]+}}(Material_{{[0-9]+}} m_{{[0-9]+}}, Texture2D<float4 > m_albedo_{{[0-9]+}}, SamplerState m_samp_{{[0-9]+}}, float2 uv_{{[0-9]+}})
