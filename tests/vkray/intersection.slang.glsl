//TEST_IGNORE_FILE:
#version 460

#extension GL_NVX_raytracing : require

struct Sphere_0
{
    vec3 position_0;
    float radius_0;
};

layout(binding = 0)
layout(std140)
uniform U_0
{
    Sphere_0 gSphere_0;
};

struct RayDesc_0
{
    vec3 Origin_0;
    float TMin_0;
    vec3 Direction_0;
    float TMax_0;
};

struct SphereHitAttributes_0
{
    vec3 normal_0;
};

bool rayIntersectsSphere_0(
    RayDesc_0 ray_0,
    Sphere_0 sphere_0,
    out float                   tHit_0,
    out SphereHitAttributes_0   attrs_0)
{
    tHit_0 = sphere_0.radius_0;
    attrs_0.normal_0 = sphere_0.position_0;
    return tHit_0 >= ray_0.TMin_0;
}

hitAttributeNVX SphereHitAttributes_0 a_0;

bool ReportHit_0(float tHit_1, uint hitKind_0, SphereHitAttributes_0 attributes_0)
{
    a_0 = attributes_0;
    bool _S1 = reportIntersectionNVX(tHit_1, hitKind_0);
    return _S1;
}

void main()
{
    RayDesc_0 ray_1;
    vec3 _S2 = gl_ObjectRayOriginNVX;

    ray_1.Origin_0 = _S2;
    vec3 _S3 = gl_ObjectRayDirectionNVX;

    ray_1.Direction_0 = _S3;
    float _S4 = gl_RayTminNVX;

    ray_1.TMin_0 = _S4;
    float _S5 = gl_RayTmaxNVX;

    ray_1.TMax_0 = _S5;

    RayDesc_0 _S6 = ray_1;

    Sphere_0 _S7 = gSphere_0;

    float _S8;
    SphereHitAttributes_0 _S9;
    bool _S10 = rayIntersectsSphere_0(_S6, _S7, _S8, _S9);

    float tHit_2 = _S8;
    SphereHitAttributes_0 attrs_1 = _S9;

    if(_S10)
    {
        bool _S11 = ReportHit_0(tHit_2, (uint((0))), attrs_1);
    }

    return;
}

