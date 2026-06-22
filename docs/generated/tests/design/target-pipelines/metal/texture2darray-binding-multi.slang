//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=15e31694fd5ee22d184f1b99f92912686150441a720712b25e415c394b7ce97f
//META: purpose=Multiple texture variants of different ranks (Texture1D / Texture3D / TextureCube / Texture2DArray) all share the positional [[texture(N)]] slot space and bind independently of buffers.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the doc: Metal's `[[texture(N)]]` is a positional slot
// space shared by every Slang texture variant. Declaration order
// determines slot index; the [[buffer(...)]] indices form a
// separate space.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

Texture1D<float4> tex1d;
Texture3D<float4> tex3d;
TextureCube<float4> texCube;
Texture2DArray<float4> tex2dArr;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float4 a = tex1d.SampleLevel(samp, float(tid.x), 0);
    float4 b = tex3d.SampleLevel(samp, float3(tid), 0);
    float4 c = texCube.SampleLevel(samp, float3(tid), 0);
    float4 d = tex2dArr.SampleLevel(samp, float3(tid), 0);
    outBuf[tid.x] = a + b + c + d;
}

// CHECK: texture1d<float
// CHECK: texture3d<float
// CHECK: texturecube<float
// CHECK: texture2d_array<float
// CHECK: texture(0)
// CHECK: texture(1)
// CHECK: texture(2)
// CHECK: texture(3)
