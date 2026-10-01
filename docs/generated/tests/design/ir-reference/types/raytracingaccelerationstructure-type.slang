//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=18390b63de1003330aaf1c938cc3b9a195ccaebbb584bbb2edb3db5df8f75c02
//META: purpose=A uniform RaytracingAccelerationStructure declaration lowers to a global_param of IR type RaytracingAccelerationStructure.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A uniform RaytracingAccelerationStructure declaration lowers to a
// global_param of IR type RaytracingAccelerationStructure. A `uniform
// RaytracingAccelerationStructure` declaration surfaces
// `RaytracingAccelerationStructure` as the IR type of the
// `global_param`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform RaytracingAccelerationStructure accel;
uniform RWStructuredBuffer<uint> outBuffer;

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    RayQuery<RAY_FLAG_NONE> rayQuery;
    RayDesc rayDesc;
    rayDesc.Origin = float3(0, 0, 0);
    rayDesc.Direction = float3(0, 0, 1);
    rayDesc.TMin = 0.0;
    rayDesc.TMax = 1.0;
    rayQuery.TraceRayInline(accel, RAY_FLAG_NONE, 0xFF, rayDesc);
    outBuffer[0] = dispatchThreadID.x;
}

// IR: let {{.*}}%accel{{.*}}: RaytracingAccelerationStructure{{.*}}= global_param
