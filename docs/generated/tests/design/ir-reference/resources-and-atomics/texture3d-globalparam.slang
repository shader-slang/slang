//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:40:00+00:00
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#shader-io
//META: doc_section_digest=8ae4aee9e798a45d3d949726815fc6daf4a5c5bd25c7402ac42fe2646d68088c
//META: purpose=A module-scope `Texture3D` parameter surfaces as a `global_param` whose result type is `TextureType(..., TextureShape3DType, ...)`.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the `TextureShape3DType` carrier on a module-scope
// `Texture3D` parameter's `global_param` result type.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform Texture3D<float4> tex;
uniform RWStructuredBuffer<int> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outbuf[0] = (int)tex.Load(int4(0, 0, 0, 0)).x;
}

// IR: %tex{{.*}}: TextureType({{.*}}TextureShape3DType{{.*}}= global_param
