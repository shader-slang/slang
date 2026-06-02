//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#meshoutputref
//META: doc_section_digest=4f2d77f0c3719656c50a0a149420cbc2adf75e40f2372c446c7a976a5ae733e9
//META: purpose=A mesh-shader entry point on `-target metal` writes to `out vertices`, `out primitives`, and `out indices` arrays through the documented `metalSetVertex`, `metalSetPrimitive`, and `metalSetIndices` opcodes in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's mesh-shader-outputs table lists `metalSetVertex`,
// `metalSetPrimitive`, and `metalSetIndices` as Metal-specific
// output-write opcodes. The portable surface is the same `out vertices`
// / `out primitives` / `out indices` parameters of the mesh entry
// point; the `set` accessor's `__target_switch` selects the
// Metal-specific opcode when lowering to Metal.

//TEST:SIMPLE(filecheck=IR):-target metal -dump-ir -o /dev/null -stage mesh -entry main

struct MeshVertex
{
    float4 pos : SV_Position;
    float3 color : COLOR;
};

struct MeshPrim
{
    uint primId : SV_PrimitiveID;
};

[shader("mesh")]
[outputtopology("triangle")]
[numthreads(3, 1, 1)]
void main(
    uint tid : SV_DispatchThreadID,
    out indices uint3 triangles[1],
    out vertices MeshVertex verts[3],
    out primitives MeshPrim prims[1])
{
    SetMeshOutputCounts(3, 1);
    MeshVertex v;
    v.pos = float4(float(tid), 0.0, 0.0, 1.0);
    v.color = float3(float(tid), 1.0, 0.0);
    verts[tid] = v;

    MeshPrim p;
    p.primId = 42;
    if (tid == 0)
    {
        prims[0] = p;
        triangles[0] = uint3(0, 1, 2);
    }
}

// IR-DAG: metalSetVertex
// IR-DAG: metalSetPrimitive
// IR-DAG: metalSetIndices
