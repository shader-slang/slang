// Keep uniform interface parameters working alongside ray payloads and attributes.
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -conformance "Material:IMaterial=0"

[anyValueSize(16)]
interface IMaterial
{
    float getValue();
}

struct Material : IMaterial
{
    float value;
    float getValue()
    {
        return value;
    }
}

struct Payload
{
    float value;
}

[shader("closesthit")]
void main(
    inout Payload payload,
    BuiltInTriangleIntersectionAttributes attributes,
    uniform IMaterial material)
{
    payload.value = material.getValue() + attributes.barycentrics.x;
}

// CHECK-DAG: OpTypePointer IncomingRayPayloadKHR
// CHECK-DAG: OpTypePointer HitAttributeKHR
// CHECK: OpFAdd
// CHECK: OpStore
