//TEST:SIMPLE(filecheck=HALF3): -target cuda -entry half3Miss -stage miss
//TEST:SIMPLE(filecheck=HALF4): -target cuda -entry half4Miss -stage miss
//TEST:SIMPLE(filecheck=NESTED): -target cuda -entry nestedMiss -stage miss
//TEST:SIMPLE(filecheck=EMPTY): -target cuda -entry emptyMiss -stage miss
//TEST:SIMPLE(filecheck=EMPTY-SENDER): -target cuda -entry emptyRayGen -stage raygeneration

// The CUDA prelude copies payloads into registers with `memcpy`, so the receiver must use the
// exact layout of the emitted CUDA C++ types. The first three payloads expose places where a
// sequential scalar walk disagrees with that native representation. The final case records how an
// empty semantic payload is removed before this ABI is selected.

struct Half3Payload
{
    half3 first;
    half3 second;
    uint16_t tail;
}

// The prelude declares half3 with size/alignment 8/4. `second` therefore starts at byte 8 and
// `tail` at byte 16 (payload register 4). Treating half3 as an ordinary 6/2 vector puts `tail` in
// register 3 instead.
// HALF3-LABEL: __miss__half3Miss()
// HALF3-NOT: getOptiXRayPayloadPtr
// HALF3: optixGetPayload_4()
// HALF3: optixSetPayload_4
// HALF3-NOT: getOptiXRayPayloadPtr
[shader("miss")]
void half3Miss(inout Half3Payload payload)
{
    payload.tail += uint16_t(payload.second.z);
}

struct Half4Payload
{
    uint head;
    half4 value;
    uint tail;
}

// The prelude declares half4 with alignment 4, not the generic vector alignment 8. Its native
// offset is byte 4 and `tail` occupies register 3. The old compiler layout instead read register 4,
// beyond the four registers supplied by the sender.
// HALF4-LABEL: __miss__half4Miss()
// HALF4-NOT: getOptiXRayPayloadPtr
// HALF4: optixGetPayload_3()
// HALF4-NOT: optixGetPayload_4()
// HALF4: optixSetPayload_3
// HALF4-NOT: optixSetPayload_4
// HALF4-NOT: getOptiXRayPayloadPtr
[shader("miss")]
void half4Miss(inout Half4Payload payload)
{
    payload.tail += uint(payload.value.w);
}

struct TailPaddedElement
{
    float first;
    double wide;
    float last;
}

struct NestedPayload
{
    TailPaddedElement values[2];
}

// `TailPaddedElement` has native size/alignment 24/8. Array element 1 therefore starts at byte 24,
// and its `last` field occupies register 10. A recursive cursor that stops at the final field's end
// starts the second element at byte 20 and reads register 8 instead.
// NESTED-LABEL: __miss__nestedMiss()
// NESTED-NOT: getOptiXRayPayloadPtr
// NESTED: optixGetPayload_10()
// NESTED: optixSetPayload_10
// NESTED-NOT: getOptiXRayPayloadPtr
[shader("miss")]
void nestedMiss(inout NestedPayload payload)
{
    payload.values[1].last += 1.0;
}

public struct EmptyPayload
{}

RaytracingAccelerationStructure scene;

// Empty payload types are eliminated before OptiX payload legalization. Both sides must therefore
// select the zero-register overload: the ray-generation shader calls `optixTrace` directly and the
// miss shader reads no payload register. No empty native C++ object crosses this ABI boundary.
// EMPTY-LABEL: __miss__emptyMiss()
// EMPTY-NOT: getOptiXRayPayloadPtr
// EMPTY-NOT: optixGetPayload_
// EMPTY-NOT: getOptiXRayPayloadPtr
[shader("miss")]
void emptyMiss(inout EmptyPayload payload)
{}

// EMPTY-SENDER-LABEL: __raygen__emptyRayGen()
// EMPTY-SENDER-NOT: PayloadRegisters
// EMPTY-SENDER-NOT: optixTraceWithRegs
// EMPTY-SENDER: optixTrace(
// EMPTY-SENDER-NOT: PayloadRegisters
// EMPTY-SENDER-NOT: optixTraceWithRegs
[shader("raygeneration")]
void emptyRayGen()
{
    EmptyPayload payload = {};
    RayDesc ray = {};
    TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, payload);
}
