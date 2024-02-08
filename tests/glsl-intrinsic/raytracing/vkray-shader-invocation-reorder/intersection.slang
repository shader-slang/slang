// intersection.slang
//TEST:SIMPLE(filecheck=CHECK_GLSL): -allow-glsl -profile glsl_460+GL_NV_ray_tracing -stage intersection -entry main -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPIRV): -allow-glsl -emit-spirv-directly -stage intersection -entry main -target spirv-assembly

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

// CHECK_GLSL: void main(
// CHECK_SPIRV: OpEntryPoint
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