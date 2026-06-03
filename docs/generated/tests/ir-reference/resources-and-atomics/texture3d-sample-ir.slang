//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:40:00+00:00
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#sampling-and-combined-samplers
//META: doc_section_digest=e5b1bf41d1ca3cba41f8873f582c350ed520a673b9c5a9a6f8cb50743ba24530
//META: purpose=`Texture3D::Sample(samp, uv)` surfaces in `main` as a `call specialize(%Sample, ..., TextureShape3DType, ...)(%tex, %samp, %uv)` against the library-level `%Sample` function.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Same pattern as the 1D variant but with `TextureShape3DType`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform Texture3D<float4> tex;
uniform SamplerState samp;
uniform RWStructuredBuffer<float4> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float3 uv = float3(tid);
    outbuf[0] = tex.Sample(samp, uv);
}

// IR-LABEL: func %main
// IR: call specialize({{.*}}TextureShape3DType{{.*}})({{.*}}%tex{{.*}}%samp{{.*}})
