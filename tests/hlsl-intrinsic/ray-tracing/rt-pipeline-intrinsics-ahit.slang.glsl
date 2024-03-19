#version 460
#extension GL_EXT_ray_tracing : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 10 0
float CheckRayDispatchValues_0()
{


    uvec3 ri_0 = ((gl_LaunchIDEXT));
    uvec3 rd_0 = ((gl_LaunchSizeEXT));

#line 20
    return float(ri_0.x) + float(ri_0.y) + float(ri_0.z) + float(rd_0.x) + float(rd_0.y) + float(rd_0.z);
}

float CheckRaySystemValues_0()
{


    vec3 wro_0 = ((gl_WorldRayOriginEXT));
    float val_0 = wro_0.x + wro_0.y + wro_0.z;

    vec3 wrd_0 = ((gl_WorldRayDirectionEXT));
    float val_1 = val_0 + wrd_0.x + wrd_0.y + wrd_0.z;

    float rayTMin_0 = ((gl_RayTminEXT));
    float val_2 = val_1 + rayTMin_0;

    float rayTCurrent_0 = ((gl_RayTmaxEXT));
    float val_3 = val_2 + rayTCurrent_0;

    uint rayFlags_0 = ((gl_IncomingRayFlagsEXT));

#line 39
    float val_4;
    switch(rayFlags_0)
    {
    case 0U:
    case 1U:
    case 2U:
    case 4U:
    case 8U:
    case 16U:
    case 32U:
    case 64U:
    case 128U:
    case 256U:
    case 512U:
        {

#line 40
            val_4 = val_3 + 1.0;

#line 40
            break;
        }
    default:
        {

#line 40
            val_4 = val_3;

#line 40
            break;
        }
    }

#line 60
    return val_4;
}

float CheckObjectSpaceSystemValues_0()
{


    uint _S1 = ((gl_InstanceID));

#line 67
    float _S2 = float(_S1);
    uint _S3 = ((gl_InstanceCustomIndexEXT));

#line 68
    float val_5 = _S2 + float(_S3);
    uint _S4 = ((gl_GeometryIndexEXT));

#line 69
    float val_6 = val_5 + float(_S4);
    uint _S5 = ((gl_PrimitiveID));

#line 70
    float val_7 = val_6 + float(_S5);

    vec3 oro_0 = ((gl_ObjectRayOriginEXT));
    float val_8 = val_7 + oro_0.x + oro_0.y + oro_0.z;

    vec3 ord_0 = ((gl_ObjectRayDirectionEXT));
    float val_9 = val_8 + ord_0.x + ord_0.y + ord_0.z;

    mat3x4 f3x4_0 = (transpose(gl_ObjectToWorldEXT));


    float val_10 = val_9 + float(f3x4_0[0][0]) + float(f3x4_0[0][1]) + float(f3x4_0[0][2]) + float(f3x4_0[0][3]) + float(f3x4_0[1][0]) + float(f3x4_0[1][1]) + float(f3x4_0[1][2]) + float(f3x4_0[1][3]) + float(f3x4_0[2][0]) + float(f3x4_0[2][1]) + float(f3x4_0[2][2]) + float(f3x4_0[2][3]);

    mat4x3 f4x3_0 = ((gl_ObjectToWorldEXT));



    float val_11 = val_10 + float(f4x3_0[0][0]) + float(f4x3_0[0][1]) + float(f4x3_0[0][2]) + float(f4x3_0[1][0]) + float(f4x3_0[1][1]) + float(f4x3_0[1][2]) + float(f4x3_0[2][0]) + float(f4x3_0[2][1]) + float(f4x3_0[2][2]) + float(f4x3_0[3][0]) + float(f4x3_0[3][1]) + float(f4x3_0[3][2]);

    mat3x4 f3x4_1 = (transpose(gl_WorldToObjectEXT));


    float val_12 = val_11 + float(f3x4_1[0][0]) + float(f3x4_1[0][1]) + float(f3x4_1[0][2]) + float(f3x4_1[0][3]) + float(f3x4_1[1][0]) + float(f3x4_1[1][1]) + float(f3x4_1[1][2]) + float(f3x4_1[1][3]) + float(f3x4_1[2][0]) + float(f3x4_1[2][1]) + float(f3x4_1[2][2]) + float(f3x4_1[2][3]);

    mat4x3 f4x3_1 = ((gl_WorldToObjectEXT));

#line 100
    return val_12 + float(f4x3_1[0][0]) + float(f4x3_1[0][1]) + float(f4x3_1[0][2]) + float(f4x3_1[1][0]) + float(f4x3_1[1][1]) + float(f4x3_1[1][2]) + float(f4x3_1[2][0]) + float(f4x3_1[2][1]) + float(f4x3_1[2][2]) + float(f4x3_1[3][0]) + float(f4x3_1[3][1]) + float(f4x3_1[3][2]);
}

float CheckHitSpecificSystemValues_0()
{


    uint hitKind_0 = ((gl_HitKindEXT));

#line 107
    float val_13;
    if(hitKind_0 == 254U || hitKind_0 == 255U)
    {

#line 108
        val_13 = 1.0;

#line 108
    }
    else
    {

#line 108
        val_13 = 0.0;

#line 108
    }

#line 113
    return val_13;
}

float CheckSysValueIntrinsics_0()
{


    float _S6 = CheckRayDispatchValues_0();
    float _S7 = CheckRaySystemValues_0();

#line 121
    float val_14 = _S6 + _S7;
    float _S8 = CheckObjectSpaceSystemValues_0();

#line 122
    float val_15 = val_14 + _S8;
    float _S9 = CheckHitSpecificSystemValues_0();

    return val_15 + _S9;
}


#line 5
struct RayPayload_0
{
    float RayHitT_0;
};


#line 129
rayPayloadInEXT RayPayload_0 _S10;


#line 9569 1
struct BuiltInTriangleIntersectionAttributes_0
{
    vec2 barycentrics_0;
};


#line 9569
hitAttributeEXT BuiltInTriangleIntersectionAttributes_0 _S11;


#line 129 0
void main()
{


    float _S12 = CheckSysValueIntrinsics_0();

    float _S13 = _S11.barycentrics_0.x;

#line 135
    float _S14 = _S11.barycentrics_0.y;
    float val_16 = _S12 + (1.0 - _S13 - _S14) + _S13 + _S14;

    vec3 _S15 = ((gl_ObjectRayOriginEXT));

#line 138
    vec3 _S16 = ((gl_ObjectRayDirectionEXT));

#line 138
    float _S17 = ((gl_RayTmaxEXT));

#line 138
    vec3 hitLocation_0 = _S15 + _S16 * _S17;

    if(hitLocation_0.x > 0.0)
    {
        _S10.RayHitT_0 = val_16;
        terminateRayEXT;;

#line 140
    }
    else
    {


        if(hitLocation_0.y < 0.0)
        {
            _S10.RayHitT_0 = val_16;
            ignoreIntersectionEXT;;

#line 145
        }

#line 140
    }

#line 151
    _S10.RayHitT_0 = val_16;
    return;
}

