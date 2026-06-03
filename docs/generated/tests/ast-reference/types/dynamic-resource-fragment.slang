//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ast-reference/types.md#resource--texture--sampler-types
//META: doc_section_digest=1fea6aaeec3d54a63a589bef03489a7dd7676c39ed93549751670b987cd46065
//META: purpose=A DynamicResourceType is the AST type for the bindless `__DynamicResource` handle; the GLSL backend lowers the surface spelling to a binding declaration in a fragment entry point.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `DynamicResourceType` is the `BuiltinType` leaf for the
// `__DynamicResource` user spelling (bindless / dynamic-resource
// handle). Observable via GLSL emit at a fragment entry point: the
// handle reaches the binding-decl machinery and the resolved
// resource (here `Texture2D`) appears in the emitted text.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -stage fragment -entry main

[vk::binding(0, 0)]
__DynamicResource handle;

// GLSL emit drops `set = 0` for the default descriptor set; only
// `binding = 0` appears in the layout qualifier here.
//CHECK-DAG: layout(binding = 0)
//CHECK-DAG: texture2D
float4 main() : SV_Target
{
    let tex = handle.as<Texture2D<float4>>();
    return tex.Load(int3(0, 0, 0));
}
