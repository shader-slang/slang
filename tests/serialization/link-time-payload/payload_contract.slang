public interface IRayTracer
{
    associatedtype Payload;
    static void hit(inout Payload payload, float2 uv);
}

public extern struct RayTracer : IRayTracer;

[shader("closesthit")]
void closest_hit(inout RayTracer.Payload payload, BuiltInTriangleIntersectionAttributes attributes)
{
    RayTracer.hit(payload, attributes.barycentrics);
}

[shader("closesthit")]
void explicit_semantics_hit(
    inout RayTracer.Payload payload : SV_RayPayload,
    BuiltInTriangleIntersectionAttributes attributes : SV_IntersectionAttributes)
{
    RayTracer.hit(payload, attributes.barycentrics);
}
