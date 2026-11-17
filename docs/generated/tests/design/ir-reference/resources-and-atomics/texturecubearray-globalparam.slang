//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#shader-io
//META: doc_section_digest=a773e5d9549bbb68f3cb1b14f6a75cb5ac368be4037f672f043b86eb02481ba9
//META: purpose=A module-scope `TextureCubeArray` parameter surfaces as a `global_param` whose result type is `TextureType(..., TextureShapeCubeDType, 1, ...)`, with the array-slice flag set.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `TextureCubeArray` shares the cube shape token with `TextureCube`
// but flips the array flag to `1`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform TextureCubeArray<float4> tex;
uniform SamplerState samp;
uniform RWStructuredBuffer<float4> outbuf;

[numthreads(4, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outbuf[0] = tex.Sample(samp, float4(0, 0, 1, 0));
}

// IR: %tex{{.*}}: TextureType({{.*}}TextureShapeCubeDType, 1 : Int{{.*}}= global_param
