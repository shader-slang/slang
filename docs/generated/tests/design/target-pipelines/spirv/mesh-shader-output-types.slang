//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0e5b4c31fb3cb504b87417d8391d1c84f2175e1be41f81660fa7556568fce3ac
//META: purpose=A mesh entry point with `out vertices` and `out indices` parameters exercises legalizeMeshOutputTypes — the SPIR-V emit declares the MeshShadingEXT capability and an OpEntryPoint MeshEXT with a Position-decorated per-vertex output.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: legalizeMeshOutputTypes (Phase C, row 20, gated on
// reqSet.meshOutput) rewrites the mesh-shader `out vertices` /
// `out indices` parameters into the SPIR-V mesh-shader output
// arrays. The SPIR-V emit declares the MeshShadingEXT capability
// and the MeshEXT execution model. SV_Position on the per-vertex
// output structure lowers to the gl_Position BuiltIn.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage mesh

struct Vert
{
    float4 pos : SV_Position;
};

[shader("mesh")]
[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(out vertices Vert verts[3], out indices uint3 tris[1])
{
    SetMeshOutputCounts(3, 1);
    verts[0].pos = float4(0, 0, 0, 1);
    verts[1].pos = float4(1, 0, 0, 1);
    verts[2].pos = float4(0, 1, 0, 1);
    tris[0] = uint3(0, 1, 2);
}

// CHECK-DAG: OpCapability MeshShadingEXT
// CHECK: OpEntryPoint MeshEXT %main
// CHECK-DAG: BuiltIn Position
