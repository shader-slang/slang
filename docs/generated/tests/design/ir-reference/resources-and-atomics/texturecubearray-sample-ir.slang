//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:40:00+00:00
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#sampling-and-combined-samplers
//META: doc_section_digest=e5b1bf41d1ca3cba41f8873f582c350ed520a673b9c5a9a6f8cb50743ba24530
//META: purpose=`TextureCubeArray::Sample(samp, uvwi)` surfaces in `main` as a `call specialize(%Sample, ..., TextureShapeCubeDType, 1, ...)` with the array flag set.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Cube array variant of the `Sample` `call specialize(...)` shape.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform TextureCubeArray<float4> tex;
uniform SamplerState samp;
uniform RWStructuredBuffer<float4> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float4 uvwi = float4(float3(tid), 0);
    outbuf[0] = tex.Sample(samp, uvwi);
}

// IR-LABEL: func %main
// IR: call specialize({{.*}}TextureShapeCubeDType, 1 : Int{{.*}})({{.*}}%tex{{.*}}%samp{{.*}})
