//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#interpolation-and-shader-io
//META: doc_section_digest=4430d3efb9516cd383356961351a223b29ad1c948e7c5ea9bf0a36d832a4b940
//META: purpose=Boundary - a `sample` modifier produces interpolationMode with mode operand 4, the upper edge of the documented mode-enum range observed in the IR dump.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary - the doc does not document the interpolationMode
// integer encoding (this is in the BUNDLE doc gaps), but the
// observed mapping is linear=0, noperspective=1, nointerpolation=2,
// centroid=3, sample=4. This test pins the upper edge.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<float> buf;

struct V
{
    sample float u : TEXCOORD0;
};

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID, V v)
{
    buf[0] = v.u + float(tid.x);
}

// CHECK: [interpolationMode(4 : Int)]
