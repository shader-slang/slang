//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:40:00+00:00
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#shader-io
//META: doc_section_digest=8ae4aee9e798a45d3d949726815fc6daf4a5c5bd25c7402ac42fe2646d68088c
//META: purpose=A module-scope `Texture1D` parameter surfaces as a `global_param` whose result type is `TextureType(..., TextureShape1DType, ...)`.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's Shader IO row says `global_param` is the opcode for a
// module-scope shader parameter, and the texture-and-image table
// enumerates the texture-shape variants. This test pins down the
// `TextureShape1DType` carrier on the result type of `global_param`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform Texture1D<float4> tex;
uniform RWStructuredBuffer<int> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outbuf[0] = (int)tex.Load(int2(0, 0)).x;
}

// IR: %tex{{.*}}: TextureType({{.*}}TextureShape1DType{{.*}}= global_param
