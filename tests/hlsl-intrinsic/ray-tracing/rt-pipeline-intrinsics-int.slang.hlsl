#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif
#pragma warning(disable: 3557)


#line 6 "D:/slang/tests/hlsl-intrinsic/ray-tracing/rt-pipeline-intrinsics-int.slang"
float CheckRayDispatchValues_0()
{


    uint3 ri_0 = DispatchRaysIndex();
    uint3 rd_0 = DispatchRaysDimensions();

#line 16
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

#line 35
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

#line 36
            val_4 = val_3 + 1.0;

#line 36
            break;
        }
    default:
        {

#line 36
            val_4 = val_3;

#line 36
            break;
        }
    }

#line 56
    return val_4;
}

float CheckObjectSpaceSystemValues_0()
{


    uint _S1 = InstanceIndex();

#line 63
    float _S2 = float(_S1);
    uint _S3 = InstanceID();

#line 64
    float val_5 = _S2 + float(_S3);
    uint _S4 = GeometryIndex();

#line 65
    float val_6 = val_5 + float(_S4);
    uint _S5 = PrimitiveIndex();

#line 66
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

#line 96
    return val_12 + (float) f4x3_1[int(0)][int(0)] + (float) f4x3_1[int(0)][int(1)] + (float) f4x3_1[int(0)][int(2)] + (float) f4x3_1[int(1)][int(0)] + (float) f4x3_1[int(1)][int(1)] + (float) f4x3_1[int(1)][int(2)] + (float) f4x3_1[int(2)][int(0)] + (float) f4x3_1[int(2)][int(1)] + (float) f4x3_1[int(2)][int(2)] + (float) f4x3_1[int(3)][int(0)] + (float) f4x3_1[int(3)][int(1)] + (float) f4x3_1[int(3)][int(2)];
}

float CheckSysValueIntrinsics_0()
{


    float _S6 = CheckRayDispatchValues_0();
    float _S7 = CheckRaySystemValues_0();

#line 104
    float val_13 = _S6 + _S7;
    float _S8 = CheckObjectSpaceSystemValues_0();

    return val_13 + _S8;
}

struct MyAttributes_0
{
    float attr_0;
};


[shader("intersection")]void main()
{

    MyAttributes_0 IDs_0;

    float3 center_0 = float3(0.0, 0.0, 1.0);


    float3 _S9 = WorldRayOrigin();

#line 124
    float3 oc_0 = _S9 - center_0;
    float3 _S10 = WorldRayDirection();

#line 125
    float3 _S11 = WorldRayDirection();

#line 125
    float a_0 = dot(_S10, _S11);
    float3 _S12 = WorldRayDirection();

#line 126
    float b_0 = dot(oc_0, _S12);

    float discriminant_0 = b_0 * b_0 - a_0 * (dot(oc_0, oc_0) - 25.0);

    float _S13 = CheckSysValueIntrinsics_0();

    if(discriminant_0 >= 0.0)
    {
        float val_14 = _S13 + discriminant_0;

        float _S14 = - b_0;

#line 136
        float _S15 = sqrt(discriminant_0);

#line 136
        float t1_0 = (_S14 - _S15) / a_0;
        float t2_0 = (_S14 + _S15) / a_0;

        float rayTMin_1 = RayTMin();
        float rayTMax_0 = RayTCurrent();
        bool _S16 = rayTMin_1 <= t1_0 && t1_0 < rayTMax_0;

#line 141
        if(_S16 || rayTMin_1 <= t2_0 && t2_0 < rayTMax_0)
        {
            IDs_0.attr_0 = val_14;

#line 143
            float _S17;
            if(_S16)
            {

#line 144
                _S17 = t1_0;

#line 144
            }
            else
            {

#line 144
                _S17 = t2_0;

#line 144
            }

#line 144
            bool _S18 = ReportHit(_S17, 0U, IDs_0);

#line 141
        }

#line 132
    }

#line 147
    return;
}

