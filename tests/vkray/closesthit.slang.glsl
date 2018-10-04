// closesthit.slang.glsl
#version 460
#extension GL_NVX_raytracing : require

layout(std430) buffer _S1
{
    vec4 colors_0[];
};

struct BuiltInTriangleIntersectionAttributes_0
{
    vec2 barycentrics_0;
};

hitAttributeNVX BuiltInTriangleIntersectionAttributes_0 _S2;

struct ReflectionRay_0
{
    vec4 color_0;
};

rayPayloadInNVX ReflectionRay_0 _S3;

void main()
{
    BuiltInTriangleIntersectionAttributes_0 _S4 = _S2;

    uint _S5 = gl_InstanceCustomIndexNVX;
    uint _S6 = gl_InstanceID;

    uint _S7 = _S5 + _S6;
    uint _S8 = gl_PrimitiveID;

    uint _S9 = _S7 + _S8;
    uint _S10 = gl_HitKindNVX;

    vec4 color_1 = colors_0[_S9 + _S10];

    float _S11 = gl_HitTNVX;
    float _S12 = gl_RayTminNVX;

    _S3.color_0 = color_1 * (_S11 - _S12);

    return;
}

