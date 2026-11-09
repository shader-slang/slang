//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#texture-and-image
//META: doc_section_digest=9181b1e074afdf780bb1918cb592389fdbe5dee019af40f1c48f4b6cb9064e6b
//META: purpose=`Texture3D::Load(int4(xyz, lod))` surfaces in `main` as a `call specialize(%Load, ..., TextureShape3DType, ...)(%tex, %coord)`.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// 3D variant of the `Load` `call specialize(...)` shape.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform Texture3D<float4> tex;
uniform RWStructuredBuffer<float4> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int4 coord = int4(int3(tid), 0);
    outbuf[0] = tex.Load(coord);
}

// IR-LABEL: func %main
// IR: call specialize({{.*}}TextureShape3DType{{.*}})({{.*}}%tex{{.*}})
