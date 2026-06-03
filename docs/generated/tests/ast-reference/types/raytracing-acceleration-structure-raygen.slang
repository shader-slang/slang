//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ast-reference/types.md#resource--texture--sampler-types
//META: doc_section_digest=1fea6aaeec3d54a63a589bef03489a7dd7676c39ed93549751670b987cd46065
//META: purpose=A RaytracingAccelerationStructureType is the AST type for `RaytracingAccelerationStructure`; the HLSL backend renders it under its documented surface name in a raygeneration entry point.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `RaytracingAccelerationStructureType` is an
// `UntypedBufferResourceType` leaf for the
// `RaytracingAccelerationStructure` user spelling. Observable via
// HLSL emit text: the keyword survives lowering at a raygeneration
// entry point.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage raygeneration -entry main

RaytracingAccelerationStructure scene;
RWStructuredBuffer<uint> outputBuffer;

struct RayPayload
{
    float4 color;
};

//CHECK: RaytracingAccelerationStructure
//CHECK: TraceRay
void main()
{
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.TMin = 0.0;
    ray.Direction = float3(0, 0, 1);
    ray.TMax = 1.0;

    RayPayload payload;
    payload.color = float4(0, 0, 0, 1);

    TraceRay(scene, 0, 0xff, 0, 0, 0, ray, payload);

    outputBuffer[0] = (uint)payload.color.x;
}
