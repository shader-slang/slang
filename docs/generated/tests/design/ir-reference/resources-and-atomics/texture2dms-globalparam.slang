//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#shader-io
//META: doc_section_digest=3ebd10067ff5f6123adbd965e4471fda38311d82079a24af41d8faab3a3f45cc
//META: purpose=A module-scope `Texture2DMS<T>` parameter surfaces as a `global_param` whose result type is `TextureType(..., TextureShape2DType, 0, 1, ...)`, with the multi-sample flag set.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `Texture2DMS<T>` flips the isMS slot (4th operand after element
// type and shape) to `1` while keeping the array flag at `0`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform Texture2DMS<float4> tex;
uniform RWStructuredBuffer<float4> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outbuf[0] = tex.Load(int2(0, 0), 0);
}

// IR: %tex{{.*}}: TextureType({{.*}}TextureShape2DType, 0 : Int, 1 : Int{{.*}}= global_param
