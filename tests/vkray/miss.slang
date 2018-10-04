//TEST:CROSS_COMPILE: -profile sm_6_3 -stage miss -entry main -target spirv-assembly

struct ShadowRay
{
    float hitDistance;
};

void main(in out ShadowRay ray)
{
	ray.hitDistance = 10000.0f;
}
