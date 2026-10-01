//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry missMain -stage miss
//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry missMain -stage miss -DMAKE_PUBLIC

// Consider this payload, which extends the shape reported in shader-slang/slang#12803 with scalar
// fields around `Empty`. Empty-type legalization removes `Empty`, but temporarily keeps its field
// as `void` so field indices still match the source type. CUDA payload lowering must treat that
// placeholder as zero storage. If it rejects the placeholder and falls back to a payload pointer,
// raygen passes inline registers while this miss shader interprets the first two registers as a
// pointer. The final 16-bit field also verifies that the placeholder adds no alignment.
#ifdef MAKE_PUBLIC
public struct Empty
{
}

public struct Payload
{
    public float3 color;
    public uint8_t low;
    public Empty empty;
    public uint16_t high;
}
#else
struct Empty
{
}

struct Payload
{
    float3 color;
    uint8_t low;
    Empty empty;
    uint16_t high;
}
#endif

// CHECK-LABEL: __miss__missMain()
// CHECK-NOT: getOptiXRayPayloadPtr
// CHECK: optixGetPayload_0
// CHECK: optixGetPayload_1
// CHECK: optixGetPayload_2
// The 8-bit and 16-bit fields share one native word. Fetch it once and reconstruct both values;
// emitting independent fetches for each source field obscures that they have one ABI location.
// CHECK: [[PACKED_REG:[A-Za-z_][A-Za-z0-9_]*]] = optixGetPayload_3()
// CHECK: uchar{{.*}}([[PACKED_REG]])
// CHECK: ushort{{.*}}([[PACKED_REG]] >> 16U)
// CHECK-NOT: getOptiXRayPayloadPtr
// The writer accumulates both subword replacements before committing the final native word.
// CHECK: optixGetPayload_3()
// CHECK: {{.*}} << 16U
// CHECK: optixSetPayload_0
// CHECK: optixSetPayload_1
// CHECK: optixSetPayload_2
// CHECK: optixSetPayload_3
// CHECK-NOT: getOptiXRayPayloadPtr
// CHECK: return;
[shader("miss")]
void missMain(inout Payload payload)
{
    payload.color = float3(1, 0, 1);
    payload.low = 1;
    payload.high = 2;
}
