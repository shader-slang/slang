//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#entrypoint--entrypointdecoration
//META: doc_section_digest=ff6c94094e2d0e2a20dc1c11d1c633e985b5a8ead5b778a7bb6d4b79b6c30013
//META: purpose=Boundary - a [shader("vertex")] entry point lowers to an entryPoint decoration whose profile integer is 1, distinct from the compute encoding.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the first operand of `entryPoint` really is a per-stage
// profile tag and not a constant: a vertex entry point pins the literal
// `1`, whereas the compute entry point in the sibling functional test
// prints a different integer. Pinning the vertex value locks the mapping
// so a renumbering of the profile enum is caught.

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
