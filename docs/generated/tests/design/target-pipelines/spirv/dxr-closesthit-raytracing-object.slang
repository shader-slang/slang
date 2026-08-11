//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0e5b4c31fb3cb504b87417d8391d1c84f2175e1be41f81660fa7556568fce3ac
//META: purpose=A DXR closesthit entry point with an inout Payload exercises replaceLocationIntrinsicsWithRaytracingObject — the payload parameter lowers to an IncomingRayPayloadKHR global variable and the SPIR-V emit declares OpCapability RayTracingKHR.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: replaceLocationIntrinsicsWithRaytracingObject (Phase C,
// row 36) is gated on isKhronosTarget && emitSpirvDirectly and fires
// for DXR entry points. The closesthit stage receives an inout
// Payload by name; on the SPIR-V emit side this is rewritten as a
// global IncomingRayPayloadKHR-storage variable that the shader
// reads/writes. The capability RayTracingKHR is declared.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage closesthit

struct Payload
{
    float3 color;
};

[shader("closesthit")]
void main(inout Payload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload.color = float3(1.0, 0.5, 0.25);
}

// CHECK-DAG: OpCapability RayTracingKHR
// CHECK-DAG: IncomingRayPayloadKHR
// CHECK-DAG: OpEntryPoint ClosestHitKHR %main
