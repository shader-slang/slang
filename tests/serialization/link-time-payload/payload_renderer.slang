import payload_contract;

public struct TestPayload
{
    public float value;
}

public struct TestRayTracer : IRayTracer
{
    public typealias Payload = TestPayload;
    public static void hit(inout TestPayload payload, float2 uv)
    {
        payload.value += uv.x;
    }
}

export struct RayTracer : IRayTracer = TestRayTracer;

[shader("closesthit")]
void concrete_hit(inout TestPayload payload, BuiltInTriangleIntersectionAttributes attributes)
{
    TestRayTracer.hit(payload, attributes.barycentrics);
}

[shader("closesthit")]
void concrete_renderer_hit(
    inout TestRayTracer.Payload payload,
    BuiltInTriangleIntersectionAttributes attributes)
{
    TestRayTracer.hit(payload, attributes.barycentrics);
}
