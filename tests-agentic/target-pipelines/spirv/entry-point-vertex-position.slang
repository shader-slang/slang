//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
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
