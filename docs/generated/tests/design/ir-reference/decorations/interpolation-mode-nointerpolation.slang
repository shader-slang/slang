//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#interpolation-and-shader-io
//META: doc_section_digest=4430d3efb9516cd383356961351a223b29ad1c948e7c5ea9bf0a36d832a4b940
//META: purpose=Boundary - a `nointerpolation` modifier produces interpolationMode with mode operand 2, the documented value distinct from `linear` (0), `noperspective` (1), `centroid` (3), and `sample` (4).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary - the existing interpolation-mode-decoration.slang
// uses `nointerpolation` but wildcards the mode integer. This
// test pins the specific value (2) for the nointerpolation mode.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<float> buf;

struct V
{
    nointerpolation float u : TEXCOORD0;
};

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID, V v)
{
    buf[0] = v.u + float(tid.x);
}

// CHECK: [interpolationMode(2 : Int)]
