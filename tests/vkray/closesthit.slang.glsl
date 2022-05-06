// closesthit.slang.glsl
#version 460
#extension GL_NV_ray_tracing : require

#define tmp_shaderrecord    _S1
#define tmp_colors          _S2
#define tmp_hitattrs        _S3
#define tmp_payload         _S4
#define tmp_customidx       _S5
#define tmp_instanceid      _S6
#define tmp_add_0           _S7
#define tmp_primid          _S8
#define tmp_add_1           _S9
#define tmp_hitkind         _S10
#define tmp_hitt            _S11
#define tmp_tmin            _S12

struct SLANG_ParameterGroup_ShaderRecord_0
{
    uint shaderRecordID_0;    
};

layout(shaderRecordNV)
buffer tmp_shaderrecord
{
    SLANG_ParameterGroup_ShaderRecord_0 _data;
} ShaderRecord_0;

layout(std430, binding = 0) readonly buffer tmp_colors
{
    vec4 _data[];
} colors_0;
 
struct BuiltInTriangleIntersectionAttributes_0
{
    vec2 barycentrics_0;
};


hitAttributeNV BuiltInTriangleIntersectionAttributes_0 tmp_hitattrs;

struct ReflectionRay_0
{
    vec4 color_0;
};

rayPayloadInNV ReflectionRay_0 tmp_payload;

void main()
{
    uint tmp_instanceid = gl_InstanceID;
    uint tmp_shift_0 = tmp_instanceid << 1;

    uint tmp_customidx = gl_InstanceCustomIndexNV;

    uint tmp_add_0 = tmp_shift_0 + tmp_customidx;
    uint tmp_primid = gl_PrimitiveID;

    uint tmp_add_1 = tmp_add_0 + tmp_primid;
    uint tmp_hitkind = gl_HitKindNV;

    vec4 color_1 = colors_0._data[tmp_add_1 + tmp_hitkind + ShaderRecord_0._data.shaderRecordID_0];

    float tmp_hitt = gl_RayTmaxNV;
    float tmp_tmin = gl_RayTminNV;

    tmp_payload.color_0 = color_1 * (tmp_hitt - tmp_tmin);

    return;
}

