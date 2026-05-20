//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/ir-reference/decorations.md#interpolation-and-shader-io
//META: doc_section_digest=4430d3efb9516cd383356961351a223b29ad1c948e7c5ea9bf0a36d832a4b940
//META: purpose=Stress - a struct with many fields, each carrying interpolationMode + glslLocation + semantic decorations, produces all per-field decorations in the dump without merging or collisions across fields.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress - sample five interpolation modes (linear=0,
// noperspective=1, nointerpolation=2, centroid=3, sample=4) on
// adjacent fields, each with a distinct TEXCOORD semantic and
// a distinct vk::location. The dump records all five
// interpolationMode mode integers, all five semantic strings,
// and the explicit per-field locations independently.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<float> buf;

struct ManyAttrs
{
    [vk::location(0)] linear           float a : TEXCOORD0;
    [vk::location(1)] noperspective    float b : TEXCOORD1;
    [vk::location(2)] nointerpolation  float c : TEXCOORD2;
    [vk::location(3)] centroid         float d : TEXCOORD3;
    [vk::location(4)] sample           float e : TEXCOORD4;
};

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID, ManyAttrs v)
{
    buf[0] = v.a + v.b + v.c + v.d + v.e + float(tid.x);
}

// CHECK-DAG: [interpolationMode(0 : Int)]
// CHECK-DAG: [interpolationMode(1 : Int)]
// CHECK-DAG: [interpolationMode(2 : Int)]
// CHECK-DAG: [interpolationMode(3 : Int)]
// CHECK-DAG: [interpolationMode(4 : Int)]
// CHECK-DAG: [glslLocation(0 : Int)]
// CHECK-DAG: [glslLocation(1 : Int)]
// CHECK-DAG: [glslLocation(2 : Int)]
// CHECK-DAG: [glslLocation(3 : Int)]
// CHECK-DAG: [glslLocation(4 : Int)]
// CHECK-DAG: [semantic("TEXCOORD0", -1 : Int)]
// CHECK-DAG: [semantic("TEXCOORD4", -1 : Int)]
