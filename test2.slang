
struct Sphere
{
    float3 position;
    float radius;
};

struct SphereHitAttributes
{
    float3 normal;
};

bool rayIntersectsSphere(
	RayDesc ray,
	Sphere sphere,
	out float tHit,
	out SphereHitAttributes attrs)
{
    // HACK: this is obviously incorrect,
    // but we just need some code for testing
    tHit = sphere.radius;
    attrs.normal = sphere.position;
    return tHit >= ray.TMin;
}


cbuffer U
{
    Sphere gSphere;
}

RWStructuredBuffer<float3> hitPositions;

[shader("intersection")]
void main1()
{
    RayDesc ray;
    ray.Origin = ObjectRayOrigin();
    ray.Direction = ObjectRayDirection();
    ray.TMin = RayTMin();
    ray.TMax = RayTCurrent();

    float tHit;
    SphereHitAttributes attrs;
    if (rayIntersectsSphere(ray, gSphere, tHit, attrs))
    {
        // ReportHit(tHit, 0, attrs);
        hitPositions[0] = ray.Origin + tHit * ray.Direction;
    }
}



[shader("anyhit")]
void main2()
{
    main1();
}
