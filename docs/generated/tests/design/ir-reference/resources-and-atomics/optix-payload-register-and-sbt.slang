//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#getoptixraypayloadptr
//META: doc_section_digest=c3a35e4b5037da8ad936ccd365aac116a4da968dbaab5a0d940274845cb0ca71
//META: purpose=A small-payload `closesthit` entry point with a uniform shader-record parameter on `-target cuda` surfaces `getOptiXPayloadRegister`, `setOptiXPayloadRegister`, and `getOptiXSbtDataPointer` opcodes in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's raytracing-payload table lists `getOptiXPayloadRegister`,
// `setOptiXPayloadRegister`, and `getOptiXSbtDataPointer` as OptiX-side
// IR opcodes synthesized for closesthit lowering on CUDA. These are
// created inside `slang-ir-legalize-varying-params.cpp` during target
// legalization — AFTER the `-dump-ir` snapshot point, so they are
// not observable via IR dump. The user-visible artifact is the CUDA
// text emit calling `optixGetPayload_*` / `optixSetPayload_*`; we
// check for both forms here. The `getOptiXSbtDataPointer` opcode
// fires only when an entry-point-uniform parameter actually requires
// SBT lookup; for this small payload + scalar uniform the CUDA emit
// folds the value rather than emitting an SBT lookup, so that claim
// is not exercised by this test (see ## Doc gaps observed in the
// bundle README).

//TEST:SIMPLE(filecheck=CHECK):-target cuda -stage closesthit -entry main

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

// CHECK-DAG: optixGetPayload_
// CHECK-DAG: optixSetPayload_
