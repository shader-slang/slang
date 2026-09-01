//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=981125a6c3119290dc85c7e2ef21bc96155d4ca3760c011f47ca6e6b10fd54c9
//META: purpose=A vertex entry point that returns SV_Position emits OpEntryPoint Vertex and lowers the position output to gl_Position with the BuiltIn Position decoration.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: vertex execution model. The varying-output
// legalization rewrites the SV_Position return as a gl_Position
// builtin output, distinct from the compute-stage globals.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage vertex

RWStructuredBuffer<float4> outBuf;

[shader("vertex")]
float4 main(uint vid : SV_VertexID) : SV_Position
{
    outBuf[vid] = float4(0, 0, 0, 1);
    return float4(float(vid), 0, 0, 1);
}

// CHECK: OpEntryPoint Vertex %main
// CHECK: OpDecorate %gl_Position BuiltIn Position
