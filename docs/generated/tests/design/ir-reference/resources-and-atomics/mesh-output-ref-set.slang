//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#meshoutputref
//META: doc_section_digest=4f2d77f0c3719656c50a0a149420cbc2adf75e40f2372c446c7a976a5ae733e9
//META: purpose=A mesh-shader entry point that writes to `out vertices`, `out primitives`, and `out indices` arrays surfaces the documented `meshOutputRef` and `meshOutputSet` opcodes in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's mesh-shader-outputs table lists `meshOutputRef(base, index)`
// as a reference to one slot of a mesh-shader output array, and
// `meshOutputSet(base, index, elementValue)` as the corresponding
// single-element write. The portable surface is the `OutputVertices`
// / `OutputIndices` / `OutputPrimitives` arrays of a `[shader("mesh")]`
// entry point: `verts[i].field = v` lowers via the `ref` accessor to
// `meshOutputRef`, while `triangles[i] = v` and `prims[i] = v` lower
// via the `set` accessor to `meshOutputSet`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage mesh -entry main

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
    if (tid < 3)
    {
        verts[tid].pos = float4(float(tid), 0.0, 0.0, 1.0);
        verts[tid].color = float3(float(tid), 1.0, 0.0);
    }
    if (tid == 0)
    {
        triangles[0] = uint3(0, 1, 2);
        prims[0].primId = 42;
    }
}

// IR-DAG: meshOutputRef
// IR-DAG: meshOutputSet
