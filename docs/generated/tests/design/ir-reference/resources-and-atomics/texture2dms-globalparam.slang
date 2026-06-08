//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:40:00+00:00
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#shader-io
//META: doc_section_digest=21e167102f1daeca19e02f308c112de6dba84ede8a917f058d59ae80138afd50
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
