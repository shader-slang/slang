//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#sampling-and-combined-samplers
//META: doc_section_digest=a0ab646bb8b7c3f2d7e5acb9f418e3f9850e9bd5e2a629c46fdeeeb725e4652c
//META: purpose=`Texture2D::Gather(samp, uv)` surfaces in `main` as a `call specialize(%Gather, ..., TextureShape2DType, ...)(%tex, %samp, %uv)`. The doc's sampling table catalogs `sample` / `sampleGrad`; `Gather` is part of the same sampling family.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `Gather` is the four-texel-fetch sampling variant from the doc's
// sampling-and-combined-samplers section.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform Texture2D<float4> tex;
uniform SamplerState samp;
uniform RWStructuredBuffer<float4> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float2 uv = float2(tid.xy);
    outbuf[0] = tex.Gather(samp, uv);
}

// IR-LABEL: func %main
// IR: call specialize({{.*}}TextureShape2DType{{.*}})({{.*}}%tex{{.*}}%samp{{.*}})
