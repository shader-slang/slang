//TEST_IGNORE_FILE:
#version 460                  

#extension GL_NV_ray_tracing : require

struct ShadowRay_0
{
	float hitDistance_0;
};

rayPayloadInNV ShadowRay_0 _S1;

void main()
{
    (_S1.hitDistance_0) = 10000.0f;
    return;
}
 