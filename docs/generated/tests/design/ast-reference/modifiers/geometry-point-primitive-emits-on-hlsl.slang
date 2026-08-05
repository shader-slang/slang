//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#hlsl-geometry-shader-input-primitive-modifiers
//META: doc_section_digest=3cb505cfd0121dfc525052f30e02d849d14355332cf36d4a1c480ef07b83b2b5
//META: purpose=The `point` geometry-shader input-primitive modifier and `[instance(N)]` are reproduced on the emitted HLSL entry point.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the input-primitive modifier selects the incoming primitive kind
// and reaches codegen attached to the parameter. A one-vertex `point` input is
// declared alongside an instanced invocation count, and the CHECK-DAG lines
// assert both the `point` qualifier on the emitted parameter and the
// `[instance(2)]` attribute survive.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -profile sm_5_0 -stage geometry -entry main

struct GsIn
{
    float4 pos : SV_Position;
};

struct GsOut
{
    float4 pos : SV_Position;
};

[maxvertexcount(1)]
[instance(2)]
void main(point GsIn input[1], inout PointStream<GsOut> outStream)
{
    GsOut o;
    o.pos = input[0].pos;
    outStream.Append(o);
}

//CHECK-DAG: [instance(2)]
//CHECK-DAG: point GsIn
