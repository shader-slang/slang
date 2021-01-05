//TEST_IGNORE_FILE:
#version 460                  

#if USE_NV_RT
#extension GL_NV_ray_tracing : require
#define callableDataInEXT callableDataInNV
#define gl_LaunchIDEXT gl_LaunchIDNV
#define hitAttributeEXT hitAttributeNV
#define ignoreIntersectionEXT ignoreIntersectionNV
#define rayPayloadInEXT rayPayloadInNV
#define terminateRayEXT terminateRayNV
#else
#extension GL_EXT_ray_tracing : require
#endif

struct ShadowRay_0
{
	float hitDistance_0;
};

rayPayloadInEXT ShadowRay_0 _S1;

void main()
{
    (_S1.hitDistance_0) = 10000.0f;
    return;
}
 