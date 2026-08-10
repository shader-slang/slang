//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=9bc7cd21838a70254771d6cf744cc574fc1ff6d4c25215b75c1badb8cf87e771
//META: purpose=A RayQuery<flags> local variable surfaces the RayQuery opcode in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A RayQuery<flags> local variable surfaces the RayQuery opcode in the
// IR dump. Declaring `RayQuery<RAY_FLAG_NONE>` in a compute entry
// point surfaces the `RayQuery` opcode in the IR dump.

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

// IR-LABEL: func %main
// IR: RayQuery
