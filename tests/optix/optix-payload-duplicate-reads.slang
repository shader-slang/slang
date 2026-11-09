//TEST:SIMPLE(filecheck=CHECK_MISS): -target cuda -entry miss -stage miss
//TEST:SIMPLE(filecheck=CHECK_CH): -target cuda -entry closest_hit -stage closesthit
//TEST:SIMPLE(filecheck=CHECK_MIX): -target cuda -entry mixed -stage closesthit

// Regression test for shader-slang/slang#12532.
//
// An `inout` ray payload that fits in registers must be read from the OptiX
// payload registers exactly once (on entry) and written back exactly once
// (before return). Previously the register-based path emitted the readback on
// BOTH the VaryingInput and the VaryingOutput legalization passes, producing a
// dead second group of reads: 8 optixGetPayload_N for a 4-register payload,
// against 4 optixSetPayload_N. The counts must be symmetric: 4 gets, 4 sets.

struct Payload
{
    float hitT;   // register 0
    float3 color; // registers 1, 2, 3
};

struct Attributes
{
    float2 barycentrics;
};

// A payload too large for the register path (40 * 16 bytes = 640 bytes > the
// 128-byte / 32-register limit), so it is legalized through the pointer-packing
// fallback and written back through a payload pointer, not registers.
struct BigPayload
{
    float4 data[40];
};

// CHECK_MISS-COUNT-4: optixGetPayload_
// CHECK_MISS-NOT: optixGetPayload_
// CHECK_MISS-COUNT-4: optixSetPayload_
// CHECK_MISS-NOT: optixSetPayload_
[shader("miss")]
void miss(inout Payload payload)
{
    payload.hitT = 0.0;
    payload.color = float3(0.0, 0.0, 0.0);
}

// CHECK_CH-COUNT-4: optixGetPayload_
// CHECK_CH-NOT: optixGetPayload_
// CHECK_CH-COUNT-4: optixSetPayload_
// CHECK_CH-NOT: optixSetPayload_
[shader("closesthit")]
void closest_hit(inout Payload payload, in Attributes attr)
{
    payload.hitT = RayTCurrent();
    payload.color = float3(attr.barycentrics.x, attr.barycentrics.y, 1.0);
}

// A register payload and a pointer-packed payload on the same entry point: the
// deduplication of the register readback must be keyed to the specific payload,
// not to a global "has any write-back been registered" flag. The register
// payload keeps its symmetric 4 gets / 4 sets, and the large payload's outgoing
// write through the payload pointer must NOT be dropped. Emit order is: the four
// register reads at entry, then the pointer-packed store, then the four register
// writes before return -- so the checks below both bound each register op at
// exactly 4 (the bug produced 8 reads) and confirm the pointer store survives.
// CHECK_MIX-COUNT-4: optixGetPayload_
// CHECK_MIX-NOT: optixGetPayload_
// CHECK_MIX: getOptiXRayPayloadPtr()){{.*}}=
// CHECK_MIX-COUNT-4: optixSetPayload_
// CHECK_MIX-NOT: optixSetPayload_
[shader("closesthit")]
void mixed(inout Payload payload, inout BigPayload big)
{
    payload.hitT = RayTCurrent();
    big.data[0] = float4(1.0, 2.0, 3.0, 4.0);
}
