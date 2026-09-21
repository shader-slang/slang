//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#sampling-and-combined-samplers
//META: doc_section_digest=796a186ea07715a784b97724aaeb7a3b60b2a410eea1fc6f2ea9cc87d5100072
//META: purpose=`Texture3D::Sample(samp, uv)` surfaces in `main` as a `call specialize(%Sample, ..., TextureShape3DType, ...)(%tex, %samp, %uv)` against the library-level `%Sample` function.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Same pattern as the 1D variant but with `TextureShape3DType`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

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
