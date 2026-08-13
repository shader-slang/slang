//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#shader-io
//META: doc_section_digest=3ebd10067ff5f6123adbd965e4471fda38311d82079a24af41d8faab3a3f45cc
//META: purpose=A module-scope `TextureCube` parameter surfaces as a `global_param` whose result type is `TextureType(..., TextureShapeCubeDType, ...)`.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the `TextureShapeCubeDType` carrier on a module-scope
// `TextureCube` parameter's `global_param` result type.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform TextureCube<float4> tex;
uniform SamplerState samp;
uniform RWStructuredBuffer<float4> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outbuf[0] = tex.Sample(samp, float3(0, 0, 1));
}

// IR: %tex{{.*}}: TextureType({{.*}}TextureShapeCubeDType{{.*}}= global_param
