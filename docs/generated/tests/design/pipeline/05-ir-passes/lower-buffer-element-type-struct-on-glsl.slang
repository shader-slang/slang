//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=857d9a7b5620991cb161cbee5d5f843c16e973d3c494110ba22777ebfea3d38f
//META: purpose=Expansion: lower-buffer-element-type handles a struct-typed buffer element — `RWStructuredBuffer<Pixel>` produces a typed SSBO whose field names survive on GLSL.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Element-type complexity axis on lower-buffer-element-type. A
// struct element survives field-by-field in the GLSL SSBO layout.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main

struct Pixel { float r; float g; float b; }

uniform float v;
uniform RWStructuredBuffer<Pixel> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Pixel p;
    p.r = v;
    p.g = v + 1.0f;
    p.b = v + 2.0f;
    buf[0] = p;
}

// GLSL: layout(std430
// GLSL-DAG: r
// GLSL-DAG: g
// GLSL-DAG: b
