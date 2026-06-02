//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#entrypoint-entrypointdecoration
//META: doc_section_digest=d3d96830534992c13d27b2f3fc509ca407eb67250f8a01585d992e27d7abbae9
//META: purpose=Boundary - a [shader("vertex")] entry point lowers to an entryPoint decoration whose profile integer (1) differs from the compute encoding (6), confirming the per-stage tag enum.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary - the doc records entryPoint with `profileInst: IRIntLit`
// but does not enumerate the per-stage integers (a BUNDLE doc gap).
// The observed encoding is vertex=1, compute=6. Pin the vertex
// edge to lock the mapping for future regressions.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage vertex

struct VOut { float4 pos : SV_Position; };

[shader("vertex")]
VOut main(uint vid : SV_VertexID)
{
    VOut o;
    o.pos = float4(float(vid), 0, 0, 1);
    return o;
}

// CHECK: [entryPoint(1 : Int, "main", "{{.*}}")]
// CHECK: func %main
