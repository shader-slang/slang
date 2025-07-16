//TEST:LANG_SERVER(filecheck=CHECK):

RWTexture2D<float4> texFrame; // Output texture
uniform float iTime; 
// In seconds
uniform float2 iResolution; // Screen size

static const float kInfinity = asfloat(0x7f800000);
static const uint32_t kIdNone = 0xFFFFFFFF;

struct Intersection
{
	float t = kInfinity;
	uint32_t id = kIdNone;
	
	bool missed() {
		return id == kIdNone;
	}
	
	[mutating]
	void update(uint32_t newId, float newT)
	{
		if(newT >= 0 && newT < t)
		{
			t = newT;
			id = newId;
		}
	}
};

struct Ray
{
	float3 origin;
	float3 direction;
	float3 at(float t)
	{
		return mad(origin, direction, float3(t));
	}
};

// Returns the t-value of the intersection of the infinite line
// with the plane given by
// `dot(planeNormal, p) == planeDist`.
float intersectPlane(Ray ray, float3 planeNormal, float planeDist)
{
	// dot(planeNormal, o + d * t) == planeDist
	// -> dot(planeNormal, o) + t * dot(planeNormal, d) == planeDist
	// -> t = (planeDist - dot(planeNormal, o)) / dot(planeNormal, d)
	return (planeDist - dot(planeNormal, ray.origin))
		/ dot(planeNormal, ray.direction);
}

float2 sortLoHi(float2 v)
{ 
	

struct OBB
{
	// Note that these are more parameters than we need;
	// technically, the sides of an OBB must all be perpendicular,
	// so there's only 3 (position) + 3 (side lengths) + 3 (rotation)
	// degrees of freedom.
	float3 corner;
	float3 edges[3];
	
	// Returns the t-value of the intersection of the OBB with
	// the ray.
	// The returned t-value may be negative; i.e. this assumes
	// the camera is outside of the box.
	// On miss, returns infinity.
	float intersect(Ray ray)
	{
		ray.origin -= corner;
		
		float2 tCloseFar;
		float tFar;
		[ForceUnroll]
		for(int i = 0; i < 3; i++)
		{
			const float3 edge = edges[i];
			const float edgeDist = dot(edge, ray.origin);
			const float factor = rcp(dot(edge, ray.direction));
			float2 slab = (float2(0, dot(edge, edge)) - edgeDist)
				* factor;
			slab = 
			if(i == 0)
			{
				tClose = min(slab.x, slab.y);
				tFar   = max(slab.x, slab.y);
			}
			else
			{
				tClose = min3(tClose, slab.x, slab.y);
				tFar = max3(tFar, slab.x, slab.y);
			}
			thisSlab = sortLoHi(thisSlab);
			tCloseFar.x = 
		}
		return kInfinity;
	}
};

[shader("compute")]
[numthreads(16, 16, 1)]
void main(uint2 thread: SV_DispatchThreadID)
{
	float2 uv = (2.0 * float2(thread) - iResolution.xy) / iResolution.y;
	
	// Right-handed Z-up coordinate system, same as Blender's
	Ray ray;
	ray.origin = float3(0, -4, 4);
	static const float kSqrtP5 = sqrt(.5);
	ray.direction = float3(
		uv.x,
		kSqrtP5 -kSqrtP5 * uv.y,
		-kSqrtP5 -kSqrtP5 * uv.y
	);
	
	Intersection intersection;
	intersection.update(0, intersectPlane(ray, float3(0,0,1), 0));
	OBB obb = OBB(float3(-1,-1,-1),{float3(2,0,0),float3(0,2,0), float3(0,0,2)});
	intersection.update(1, obb.intersect(ray));
	
	float3 color;
	if(intersection.missed())
	{
		color = float3(0.0, 0.0, 1.0);
	}
	else
	{
		color = float3(intersection.t / 10.0);
	}
//COMPLETE:134,31
	texFrame[thread] = float4(color, 1.0);
}

// CHECK: color
