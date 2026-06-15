//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:40:00+00:00
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#shader-io
//META: doc_section_digest=21e167102f1daeca19e02f308c112de6dba84ede8a917f058d59ae80138afd50
//META: purpose=A module-scope `TextureCubeArray` parameter surfaces as a `global_param` whose result type is `TextureType(..., TextureShapeCubeDType, 1, ...)`, with the array-slice flag set.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `TextureCubeArray` shares the cube shape token with `TextureCube`
// but flips the array flag to `1`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform TextureCubeArray<float4> tex;
uniform SamplerState samp;
uniform RWStructuredBuffer<float4> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outbuf[0] = tex.Sample(samp, float4(0, 0, 1, 0));
}

// IR: %tex{{.*}}: TextureType({{.*}}TextureShapeCubeDType, 1 : Int{{.*}}= global_param
