//TEST_IGNORE_FILE:
#version 460                  

#extension GL_EXT_ray_tracing : require

struct ShadowRay_0
{
	float hitDistance_0;
};

rayPayloadInEXT ShadowRay_0 ray_0;

void main()
{
    (ray_0.hitDistance_0) = 10000.0f;
    return;
}

