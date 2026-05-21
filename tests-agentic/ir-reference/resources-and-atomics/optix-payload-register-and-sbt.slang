//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/llm-generated/ir-reference/resources-and-atomics.md#getoptixraypayloadptr
//META: doc_section_digest=c3a35e4b5037da8ad936ccd365aac116a4da968dbaab5a0d940274845cb0ca71
//META: purpose=A small-payload `closesthit` entry point with a uniform shader-record parameter on `-target cuda` surfaces `getOptiXPayloadRegister`, `setOptiXPayloadRegister`, and `getOptiXSbtDataPointer` opcodes in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's raytracing-payload table lists `getOptiXPayloadRegister`,
// `setOptiXPayloadRegister`, and `getOptiXSbtDataPointer` as OptiX-side
// IR opcodes. Small payloads (<= 8 registers) use the register-based
// access path; entry-point-uniform parameters lower to the SBT-data
// pointer.

//TEST:SIMPLE(filecheck=IR):-target cuda -dump-ir -o /dev/null -stage closesthit -entry main

struct SmallPayload
{
    float hitT;
    float3 color;
};

struct ShaderRecord
{
    float scale;
};

uniform ShaderRecord sbtRecord;

[shader("closesthit")]
void main(inout SmallPayload payload, BuiltInTriangleIntersectionAttributes attribs)
{
    payload.hitT = RayTCurrent() * sbtRecord.scale;
    payload.color = float3(attribs.barycentrics, 1.0);
}

// IR-DAG: getOptiXPayloadRegister
// IR-DAG: setOptiXPayloadRegister
// IR-DAG: getOptiXSbtDataPointer
