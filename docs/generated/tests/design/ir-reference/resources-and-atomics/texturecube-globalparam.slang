//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#shader-io
//META: doc_section_digest=f7c6858573b963cd26309bca37271d80f05b56002d6f2e67fef1e445032158d0
//META: purpose=A module-scope `TextureCube` parameter surfaces as a `global_param` whose result type is `TextureType(..., TextureShapeCubeDType, ...)`.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the `TextureShapeCubeDType` carrier on a module-scope
// `TextureCube` parameter's `global_param` result type.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform TextureCube<float4> tex;
uniform SamplerState samp;
uniform RWStructuredBuffer<float4> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outbuf[0] = tex.Sample(samp, float3(0, 0, 1));
}

// IR: %tex{{.*}}: TextureType({{.*}}TextureShapeCubeDType{{.*}}= global_param
