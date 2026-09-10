//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-fragdepth-and-geometry-primitive-refinements
//META: doc_section_digest=f52a7151fb266a0395ad9852aa535a328de8a8e0c05ff15ae8b0008b93f3fcaf
//META: purpose=A geometry entry point that declares no input primitive qualifier falls back to the triangle default, emitting OpExecutionMode Triangles.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the default-input-primitive fallback that now runs at the end of
// entry-point legalization, after every parameter has been processed. The
// input vertex array carries no point / line / triangle qualifier, so the
// fallback must supply one; CHECK asserts the emitted execution mode is
// Triangles alongside the geometry capability, which is the companion case to
// the test that pins an explicitly qualified point input.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage geometry

struct GsIn
{
    float4 pos : SV_Position;
}

struct GsOut
{
    float4 pos : SV_Position;
}

[shader("geometry")]
[maxvertexcount(3)]
void main(GsIn verts[3], inout TriangleStream<GsOut> stream)
{
    GsOut v;
    v.pos = verts[0].pos;
    stream.Append(v);
}

// CHECK-DAG: OpCapability Geometry
// CHECK-DAG: OpExecutionMode %{{[a-zA-Z_0-9]+}} Triangles
