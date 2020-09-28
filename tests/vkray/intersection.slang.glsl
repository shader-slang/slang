//TEST_IGNORE_FILE:
#version 460

#extension GL_NV_ray_tracing : require

#define tmp_ubo _S1
#define tmp_reportHit _S2
#define tmp_origin _S3
#define tmp_direction _S4
#define tmp_tmin _S5
#define tmp_tmax _S6
#define tmp_thit _S7
#define tmp_hitattrs _S8
#define tmp_dithit _S9
#define tmp_reportresult _S10

struct Sphere_0
{
    vec3 position_0;
    float radius_0;
};

struct SLANG_ParameterGroup_U_0
{
    Sphere_0 gSphere_0;
};

layout(binding = 0)
layout(std140)
uniform tmp_ubo
{
    SLANG_ParameterGroup_U_0 _data;
} U_0;

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

hitAttributeNV SphereHitAttributes_0 a_0;

bool ReportHit_0(float tHit_1, uint hitKind_0, SphereHitAttributes_0 attributes_0)
{
    a_0 = attributes_0;
    bool tmp_reportHit = reportIntersectionNV(tHit_1, hitKind_0);
    return tmp_reportHit;
}

void main()
{
    RayDesc_0 ray_1;

    vec3 tmp_origin = gl_ObjectRayOriginNV;
    ray_1.Origin_0 = tmp_origin;

    vec3 tmp_direction = gl_ObjectRayDirectionNV;
    ray_1.Direction_0 = tmp_direction;

    float tmp_tmin = gl_RayTminNV;
    ray_1.TMin_0 = tmp_tmin;

    float tmp_tmax = gl_RayTmaxNV;
    ray_1.TMax_0 = tmp_tmax;

    float tmp_thit;
    SphereHitAttributes_0 tmp_hitattrs;
    bool tmp_dithit = rayIntersectsSphere_0(ray_1, U_0._data.gSphere_0, tmp_thit, tmp_hitattrs);

    float tHit_2 = tmp_thit;
    SphereHitAttributes_0 attrs_1 = tmp_hitattrs;

    if(tmp_dithit)
    {
        bool tmp_reportresult = ReportHit_0(tHit_2, (uint((0))), attrs_1);
    }

    return;
}

