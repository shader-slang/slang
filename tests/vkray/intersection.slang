// intersection.slang
//TEST:CROSS_COMPILE: -profile glsl_460+GL_NV_ray_tracing -stage intersection -entry main -target spirv-assembly

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
	RayDesc 				ray,
	Sphere 					sphere,
	out float 				tHit,
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

void main()
{
	RayDesc ray;
	ray.Origin = ObjectRayOrigin();
	ray.Direction = ObjectRayDirection();
	ray.TMin = RayTMin();
	ray.TMax = RayTCurrent();

	float tHit;
	SphereHitAttributes attrs;
	if(rayIntersectsSphere(ray, gSphere, tHit, attrs))
	{
		ReportHit(tHit, 0, attrs);
	}
}
