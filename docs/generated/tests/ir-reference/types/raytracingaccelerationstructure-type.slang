//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=094f3ccbff16c121504cf60cd04ae15b937c78df94da8d568013fe4104a433b9
//META: purpose=A uniform RaytracingAccelerationStructure declaration lowers to a global_param of IR type RaytracingAccelerationStructure.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Resource and texture types" table lists
// `RaytracingAccelerationStructure` (C++ wrapper
// `RaytracingAccelerationStructureType`) as a no-operand opcode for
// the raytracing acceleration-structure resource. A
// `uniform RaytracingAccelerationStructure` declaration surfaces
// `RaytracingAccelerationStructure` as the IR type of the
// `global_param`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

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
