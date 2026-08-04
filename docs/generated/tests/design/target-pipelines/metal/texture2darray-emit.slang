//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=3b19519e792b092406320b661c62d171b0f8cdff6f282ae48cf30ac091136ae8
//META: purpose=Texture2DArray<float4> emits as metal::texture2d_array<float, access::sample> with a positional [[texture(N)]] binding.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Metal emitter maps Slang's Texture2DArray<float4> to the
// Metal texture2d_array<float, access::sample> template
// instantiation (the `_array` suffix is Metal's spelling for
// array-of-texture).

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

Texture2DArray<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(samp, float3(tid), 0);
}

// CHECK: texture2d_array<float
// CHECK: texture(0)
