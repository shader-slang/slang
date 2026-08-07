//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#mesh-shader-outputs
//META: doc_section_digest=bedd4b56e7e5495dfd99f9d6eab0f36c5057ff4dadfc06a01626d0c188d9d28a
//META: purpose=A mesh-shader entry point on `-target metal` writes to `out vertices`, `out primitives`, and `out indices` arrays through the documented `metalSetVertex`, `metalSetPrimitive`, and `metalSetIndices` opcodes in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the Metal-specific mesh output writes. The same `out
// vertices` / `out indices` / `out primitives` parameters are used as
// in the portable form; compiling with `-target metal` makes the
// `set` accessor's target switch select the Metal opcodes, and the
// order-independent CHECKs pin all three.

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
