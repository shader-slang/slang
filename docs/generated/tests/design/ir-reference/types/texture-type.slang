//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=8a7a3c63487b8f293c70c075134ef33eae10ff00b2e4f1c4f9b195bed7528e0c
//META: purpose=A uniform Texture2D<float4> declaration lowers to a global_param of IR type TextureType with element-type and TextureShape2DType operands.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A uniform Texture2D<float4> declaration lowers to a global_param of
// IR type TextureType with element-type and TextureShape2DType
// operands. `TextureShapeBufferType`).

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform Texture2D<float4> tex;
uniform SamplerState samp;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = tex.SampleLevel(samp, float2(0, 0), 0).x;
}

// IR: let {{.*}}%tex{{.*}}: TextureType(Vec(Float, 4 : Int), TextureShape2DType{{.*}}= global_param
