//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#shader-io
//META: doc_section_digest=212a9e3fd4ca324be91b034872f2b7a9afdb1fe8ac5acfc9f068664d69c3e326
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
