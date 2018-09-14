//TEST_IGNORE_FILE:
#version 460                  

#extension GL_NVX_raytracing : require

struct ShadowRay_0
{
	float hitDistance_0;
};

rayPayloadInNVX ShadowRay_0 _S1;

void main()
{
    (_S1.hitDistance_0) = 10000.0f;
    return;
}
 