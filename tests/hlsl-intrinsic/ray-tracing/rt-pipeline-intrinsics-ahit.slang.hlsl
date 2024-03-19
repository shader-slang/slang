#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif
#pragma warning(disable: 3557)


#line 10 "D:/slang/tests/hlsl-intrinsic/ray-tracing/rt-pipeline-intrinsics-ahit.slang"
float CheckRayDispatchValues_0()
{


    uint3 ri_0 = DispatchRaysIndex();
    uint3 rd_0 = DispatchRaysDimensions();

#line 20
    return float(ri_0.x) + float(ri_0.y) + float(ri_0.z) + float(rd_0.x) + float(rd_0.y) + float(rd_0.z);
}

float CheckRaySystemValues_0()
{


    float3 wro_0 = WorldRayOrigin();
    float val_0 = wro_0.x + wro_0.y + wro_0.z;

    float3 wrd_0 = WorldRayDirection();
    float val_1 = val_0 + wrd_0.x + wrd_0.y + wrd_0.z;

    float rayTMin_0 = RayTMin();
    float val_2 = val_1 + rayTMin_0;

    float rayTCurrent_0 = RayTCurrent();
    float val_3 = val_2 + rayTCurrent_0;

    uint rayFlags_0 = RayFlags();

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


    uint _S1 = InstanceIndex();

#line 67
    float _S2 = float(_S1);
    uint _S3 = InstanceID();

#line 68
    float val_5 = _S2 + float(_S3);
    uint _S4 = GeometryIndex();

#line 69
    float val_6 = val_5 + float(_S4);
    uint _S5 = PrimitiveIndex();

#line 70
    float val_7 = val_6 + float(_S5);

    float3 oro_0 = ObjectRayOrigin();
    float val_8 = val_7 + oro_0.x + oro_0.y + oro_0.z;

    float3 ord_0 = ObjectRayDirection();
    float val_9 = val_8 + ord_0.x + ord_0.y + ord_0.z;

    matrix<float,int(3),int(4)>  f3x4_0 = ObjectToWorld3x4();


    float val_10 = val_9 + (float) f3x4_0[int(0)][int(0)] + (float) f3x4_0[int(0)][int(1)] + (float) f3x4_0[int(0)][int(2)] + (float) f3x4_0[int(0)][int(3)] + (float) f3x4_0[int(1)][int(0)] + (float) f3x4_0[int(1)][int(1)] + (float) f3x4_0[int(1)][int(2)] + (float) f3x4_0[int(1)][int(3)] + (float) f3x4_0[int(2)][int(0)] + (float) f3x4_0[int(2)][int(1)] + (float) f3x4_0[int(2)][int(2)] + (float) f3x4_0[int(2)][int(3)];

    matrix<float,int(4),int(3)>  f4x3_0 = ObjectToWorld4x3();



    float val_11 = val_10 + (float) f4x3_0[int(0)][int(0)] + (float) f4x3_0[int(0)][int(1)] + (float) f4x3_0[int(0)][int(2)] + (float) f4x3_0[int(1)][int(0)] + (float) f4x3_0[int(1)][int(1)] + (float) f4x3_0[int(1)][int(2)] + (float) f4x3_0[int(2)][int(0)] + (float) f4x3_0[int(2)][int(1)] + (float) f4x3_0[int(2)][int(2)] + (float) f4x3_0[int(3)][int(0)] + (float) f4x3_0[int(3)][int(1)] + (float) f4x3_0[int(3)][int(2)];

    matrix<float,int(3),int(4)>  f3x4_1 = WorldToObject3x4();


    float val_12 = val_11 + (float) f3x4_1[int(0)][int(0)] + (float) f3x4_1[int(0)][int(1)] + (float) f3x4_1[int(0)][int(2)] + (float) f3x4_1[int(0)][int(3)] + (float) f3x4_1[int(1)][int(0)] + (float) f3x4_1[int(1)][int(1)] + (float) f3x4_1[int(1)][int(2)] + (float) f3x4_1[int(1)][int(3)] + (float) f3x4_1[int(2)][int(0)] + (float) f3x4_1[int(2)][int(1)] + (float) f3x4_1[int(2)][int(2)] + (float) f3x4_1[int(2)][int(3)];

    matrix<float,int(4),int(3)>  f4x3_1 = WorldToObject4x3();

#line 100
    return val_12 + (float) f4x3_1[int(0)][int(0)] + (float) f4x3_1[int(0)][int(1)] + (float) f4x3_1[int(0)][int(2)] + (float) f4x3_1[int(1)][int(0)] + (float) f4x3_1[int(1)][int(1)] + (float) f4x3_1[int(1)][int(2)] + (float) f4x3_1[int(2)][int(0)] + (float) f4x3_1[int(2)][int(1)] + (float) f4x3_1[int(2)][int(2)] + (float) f4x3_1[int(3)][int(0)] + (float) f4x3_1[int(3)][int(1)] + (float) f4x3_1[int(3)][int(2)];
}

float CheckHitSpecificSystemValues_0()
{


    uint hitKind_0 = HitKind();

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
[shader("anyhit")]void main(inout RayPayload_0 payload_0, BuiltInTriangleIntersectionAttributes attribs_0 : SV_IntersectionAttributes)
{


    float _S10 = CheckSysValueIntrinsics_0();

    float _S11 = attribs_0.barycentrics.x;

#line 135
    float _S12 = attribs_0.barycentrics.y;
    float val_16 = _S10 + (1.0 - _S11 - _S12) + _S11 + _S12;

    float3 _S13 = ObjectRayOrigin();

#line 138
    float3 _S14 = ObjectRayDirection();

#line 138
    float _S15 = RayTCurrent();

#line 138
    float3 hitLocation_0 = _S13 + _S14 * _S15;

    if(hitLocation_0.x > 0.0)
    {
        payload_0.RayHitT_0 = val_16;
        AcceptHitAndEndSearch();

#line 140
    }
    else
    {


        if(hitLocation_0.y < 0.0)
        {
            payload_0.RayHitT_0 = val_16;
            IgnoreHit();

#line 145
        }

#line 140
    }

#line 151
    payload_0.RayHitT_0 = val_16;
    return;
}

