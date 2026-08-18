//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#shader-io
//META: doc_section_digest=212a9e3fd4ca324be91b034872f2b7a9afdb1fe8ac5acfc9f068664d69c3e326
//META: purpose=A module-scope `Texture2DArray` parameter surfaces as a `global_param` whose result type is `TextureType(..., TextureShape2DType, 1, ...)`, where the `1` is the array-slice flag.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `Texture2DArray` shares the 2D shape token but flips the array
// slot in the `TextureType(...)` operand list to `1`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform Texture2DArray<float4> tex;
uniform SamplerState samp;
uniform RWStructuredBuffer<float4> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outbuf[0] = tex.Sample(samp, float3(0, 0, 0));
}

// IR: %tex{{.*}}: TextureType({{.*}}TextureShape2DType, 1 : Int{{.*}}= global_param
