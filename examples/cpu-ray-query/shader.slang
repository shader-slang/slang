struct Triangle
{
    float3 a, b, c;
    float3 albedo;
    float3 emission;
};

RaytracingAccelerationStructure scene;
StructuredBuffer<Triangle> triangles;
RWStructuredBuffer<float3> pixels;
uint imageSize;
uint samplesPerPixel;
uint lightTriangleIndex;

float random(inout uint state)
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return float(state & 0xffffff) / 16777216.0;
}

// Cosine-weighted sampling cancels the Lambertian BRDF and PDF, leaving just albedo.
float3 sampleHemisphere(float3 normal, inout uint state)
{
    float u = random(state);
    float angle = 6.2831853 * random(state);
    float3 axis = abs(normal.z) < 0.99 ? float3(0, 0, 1) : float3(0, 1, 0);
    float3 tangent = normalize(cross(axis, normal));
    float3 bitangent = cross(normal, tangent);
    return tangent * (sqrt(u) * cos(angle)) + bitangent * (sqrt(u) * sin(angle))
        + normal * sqrt(1 - u);
}

bool visible(float3 origin, float3 lightPoint)
{
    float3 offset = lightPoint - origin;
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = normalize(offset);
    ray.TMin = 0.001;
    ray.TMax = length(offset) - 0.002;
    RayQuery<RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> query;
    query.TraceRayInline(scene, RAY_FLAG_NONE, 0xff, ray);
    while (query.Proceed()) {}
    return query.CommittedStatus() == COMMITTED_NOTHING;
}

float3 tracePath(RayDesc ray, inout uint state)
{
    float3 radiance = 0;
    float3 throughput = 1;
    Triangle light = triangles[lightTriangleIndex];
    float3 lightEdge1 = light.b - light.a;
    float3 lightEdge2 = light.c - light.b;
    float3 lightCross = cross(lightEdge1, lightEdge2);
    float lightArea = length(lightCross);
    float3 lightNormal = lightCross / lightArea;
    RayQuery<RAY_FLAG_FORCE_OPAQUE> query;

    for (uint bounce = 0; bounce < 5; ++bounce)
    {
        query.TraceRayInline(scene, RAY_FLAG_NONE, 0xff, ray);
        while (query.Proceed()) {}
        if (query.CommittedStatus() != COMMITTED_TRIANGLE_HIT)
            break;
        Triangle hit = triangles[query.CommittedPrimitiveIndex()];
        float3 position = ray.Origin + ray.Direction * query.CommittedRayT();
        float3 normal = normalize(cross(hit.b - hit.a, hit.c - hit.a));
        if (dot(normal, ray.Direction) > 0)
            normal = -normal;

        if (any(hit.emission > 0))
        {
            // Later light hits are already accounted for by explicit light sampling below.
            if (bounce == 0)
                radiance += throughput * hit.emission;
            break;
        }

        float3 origin = position + normal * 0.001;
        float3 lightPoint = light.a + random(state) * lightEdge1 + random(state) * lightEdge2;
        float3 toLight = lightPoint - origin;
        float distanceSquared = dot(toLight, toLight);
        float3 lightDirection = toLight / sqrt(distanceSquared);
        float weight = saturate(dot(normal, lightDirection))
            * saturate(dot(lightNormal, -lightDirection)) * lightArea
            / (3.14159265 * distanceSquared);
        if (weight > 0 && visible(origin, lightPoint))
            radiance += throughput * hit.albedo * light.emission * weight;

        throughput *= hit.albedo;
        ray.Origin = origin;
        ray.Direction = sampleHemisphere(normal, state);
    }
    return radiance;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint pixel = tid.y * imageSize + tid.x;
    uint state = (pixel + 1) * 9781 + 0x68bc21eb;
    float3 radiance = 0;
    for (uint sample = 0; sample < samplesPerPixel; ++sample)
    {
        float2 uv = (float2(tid.xy) + float2(random(state), random(state))) / float(imageSize);
        RayDesc ray;
        ray.Origin = float3(0, 1, -3.2);
        ray.Direction = normalize(float3((uv.x * 2 - 1) * 0.36, (1 - uv.y * 2) * 0.36, 1));
        ray.TMin = 0.001;
        ray.TMax = 100;
        radiance += tracePath(ray, state);
    }
    pixels[pixel] = radiance / float(samplesPerPixel);
}
