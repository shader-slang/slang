//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:40:00+00:00
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#sampling-and-combined-samplers
//META: doc_section_digest=6abec3bf0db83e786c6abe3cd9da9484056f015e4a5f0072a103fdda00cee022
//META: purpose=`TextureCube::Sample(samp, uv)` surfaces in `main` as a `call specialize(%Sample, ..., TextureShapeCubeDType, ...)` against the library helper.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Cube variant of the `Sample` `call specialize(...)` shape.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform TextureCube<float4> tex;
uniform SamplerState samp;
uniform RWStructuredBuffer<float4> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float3 uv = float3(tid);
    outbuf[0] = tex.Sample(samp, uv);
}

// IR-LABEL: func %main
// IR: call specialize({{.*}}TextureShapeCubeDType{{.*}})({{.*}}%tex{{.*}}%samp{{.*}})
