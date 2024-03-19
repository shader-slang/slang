#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif
#pragma warning(disable: 3557)


#line 6 "D:/slang/tests/hlsl-intrinsic/ray-tracing/rt-pipeline-intrinsics-chit.slang"
RaytracingAccelerationStructure accelStruct_0 : register(t0);

struct RayPayload_0
{
    float RayHitT_0;
};


#line 127
float CheckTraceRay_0(RayPayload_0 payload_0, RayDesc rayDesc_0)
{

#line 127
    RayPayload_0 _S1 = payload_0;

#line 132
    TraceRay(accelStruct_0, 0U, 4294967295U, 0U, 1U, 0U, rayDesc_0, _S1);
    float _S2 = _S1.RayHitT_0;
    TraceRay(accelStruct_0, 1U, 4294967295U, 0U, 1U, 0U, rayDesc_0, _S1);
    float val_0 = _S2 + _S1.RayHitT_0;
    TraceRay(accelStruct_0, 2U, 4294967295U, 0U, 1U, 0U, rayDesc_0, _S1);
    float val_1 = val_0 + _S1.RayHitT_0;
    TraceRay(accelStruct_0, 4U, 4294967295U, 0U, 1U, 0U, rayDesc_0, _S1);
    float val_2 = val_1 + _S1.RayHitT_0;
    TraceRay(accelStruct_0, 8U, 4294967295U, 0U, 1U, 0U, rayDesc_0, _S1);
    float val_3 = val_2 + _S1.RayHitT_0;
    TraceRay(accelStruct_0, 16U, 4294967295U, 0U, 1U, 0U, rayDesc_0, _S1);
    float val_4 = val_3 + _S1.RayHitT_0;
    TraceRay(accelStruct_0, 32U, 4294967295U, 0U, 1U, 0U, rayDesc_0, _S1);
    float val_5 = val_4 + _S1.RayHitT_0;
    TraceRay(accelStruct_0, 64U, 4294967295U, 0U, 1U, 0U, rayDesc_0, _S1);
    float val_6 = val_5 + _S1.RayHitT_0;
    TraceRay(accelStruct_0, 128U, 4294967295U, 0U, 1U, 0U, rayDesc_0, _S1);
    float val_7 = val_6 + _S1.RayHitT_0;
    TraceRay(accelStruct_0, 256U, 4294967295U, 0U, 1U, 0U, rayDesc_0, _S1);
    float val_8 = val_7 + _S1.RayHitT_0;
    TraceRay(accelStruct_0, 512U, 4294967295U, 0U, 1U, 0U, rayDesc_0, _S1);

#line 180
    return val_8 + _S1.RayHitT_0;
}


#line 21
float CheckRayDispatchValues_0()
{


    uint3 ri_0 = DispatchRaysIndex();
    uint3 rd_0 = DispatchRaysDimensions();

#line 31
    return float(ri_0.x) + float(ri_0.y) + float(ri_0.z) + float(rd_0.x) + float(rd_0.y) + float(rd_0.z);
}

float CheckRaySystemValues_0()
{


    float3 wro_0 = WorldRayOrigin();
    float val_9 = wro_0.x + wro_0.y + wro_0.z;

    float3 wrd_0 = WorldRayDirection();
    float val_10 = val_9 + wrd_0.x + wrd_0.y + wrd_0.z;

    float rayTMin_0 = RayTMin();
    float val_11 = val_10 + rayTMin_0;

    float rayTCurrent_0 = RayTCurrent();
    float val_12 = val_11 + rayTCurrent_0;

    uint rayFlags_0 = RayFlags();

#line 50
    float val_13;
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

#line 51
            val_13 = val_12 + 1.0;

#line 51
            break;
        }
    default:
        {

#line 51
            val_13 = val_12;

#line 51
            break;
        }
    }

#line 71
    return val_13;
}

float CheckObjectSpaceSystemValues_0()
{


    uint _S3 = InstanceIndex();

#line 78
    float _S4 = float(_S3);
    uint _S5 = InstanceID();

#line 79
    float val_14 = _S4 + float(_S5);
    uint _S6 = GeometryIndex();

#line 80
    float val_15 = val_14 + float(_S6);
    uint _S7 = PrimitiveIndex();

#line 81
    float val_16 = val_15 + float(_S7);

    float3 oro_0 = ObjectRayOrigin();
    float val_17 = val_16 + oro_0.x + oro_0.y + oro_0.z;

    float3 ord_0 = ObjectRayDirection();
    float val_18 = val_17 + ord_0.x + ord_0.y + ord_0.z;

    matrix<float,int(3),int(4)>  f3x4_0 = ObjectToWorld3x4();


    float val_19 = val_18 + (float) f3x4_0[int(0)][int(0)] + (float) f3x4_0[int(0)][int(1)] + (float) f3x4_0[int(0)][int(2)] + (float) f3x4_0[int(0)][int(3)] + (float) f3x4_0[int(1)][int(0)] + (float) f3x4_0[int(1)][int(1)] + (float) f3x4_0[int(1)][int(2)] + (float) f3x4_0[int(1)][int(3)] + (float) f3x4_0[int(2)][int(0)] + (float) f3x4_0[int(2)][int(1)] + (float) f3x4_0[int(2)][int(2)] + (float) f3x4_0[int(2)][int(3)];

    matrix<float,int(4),int(3)>  f4x3_0 = ObjectToWorld4x3();



    float val_20 = val_19 + (float) f4x3_0[int(0)][int(0)] + (float) f4x3_0[int(0)][int(1)] + (float) f4x3_0[int(0)][int(2)] + (float) f4x3_0[int(1)][int(0)] + (float) f4x3_0[int(1)][int(1)] + (float) f4x3_0[int(1)][int(2)] + (float) f4x3_0[int(2)][int(0)] + (float) f4x3_0[int(2)][int(1)] + (float) f4x3_0[int(2)][int(2)] + (float) f4x3_0[int(3)][int(0)] + (float) f4x3_0[int(3)][int(1)] + (float) f4x3_0[int(3)][int(2)];

    matrix<float,int(3),int(4)>  f3x4_1 = WorldToObject3x4();


    float val_21 = val_20 + (float) f3x4_1[int(0)][int(0)] + (float) f3x4_1[int(0)][int(1)] + (float) f3x4_1[int(0)][int(2)] + (float) f3x4_1[int(0)][int(3)] + (float) f3x4_1[int(1)][int(0)] + (float) f3x4_1[int(1)][int(1)] + (float) f3x4_1[int(1)][int(2)] + (float) f3x4_1[int(1)][int(3)] + (float) f3x4_1[int(2)][int(0)] + (float) f3x4_1[int(2)][int(1)] + (float) f3x4_1[int(2)][int(2)] + (float) f3x4_1[int(2)][int(3)];

    matrix<float,int(4),int(3)>  f4x3_1 = WorldToObject4x3();

#line 111
    return val_21 + (float) f4x3_1[int(0)][int(0)] + (float) f4x3_1[int(0)][int(1)] + (float) f4x3_1[int(0)][int(2)] + (float) f4x3_1[int(1)][int(0)] + (float) f4x3_1[int(1)][int(1)] + (float) f4x3_1[int(1)][int(2)] + (float) f4x3_1[int(2)][int(0)] + (float) f4x3_1[int(2)][int(1)] + (float) f4x3_1[int(2)][int(2)] + (float) f4x3_1[int(3)][int(0)] + (float) f4x3_1[int(3)][int(1)] + (float) f4x3_1[int(3)][int(2)];
}

float CheckHitSpecificSystemValues_0()
{


    uint hitKind_0 = HitKind();

#line 118
    float val_22;
    if(hitKind_0 == 254U || hitKind_0 == 255U)
    {

#line 119
        val_22 = 1.0;

#line 119
    }
    else
    {

#line 119
        val_22 = 0.0;

#line 119
    }

#line 124
    return val_22;
}


#line 183
float CheckSysValueIntrinsics_0()
{


    float _S8 = CheckRayDispatchValues_0();
    float _S9 = CheckRaySystemValues_0();

#line 188
    float val_23 = _S8 + _S9;
    float _S10 = CheckObjectSpaceSystemValues_0();

#line 189
    float val_24 = val_23 + _S10;
    float _S11 = CheckHitSpecificSystemValues_0();

    return val_24 + _S11;
}


#line 13
struct CallableParams_0
{
    float value_0;
};


#line 196
[shader("closesthit")]void main(inout RayPayload_0 payload_1, BuiltInTriangleIntersectionAttributes attribs_0)
{
    uint3 _S12 = DispatchRaysIndex();

#line 198
    uint2 _S13 = _S12.xy;

#line 198
    uint3 _S14 = DispatchRaysDimensions();

#line 198
    uint2 _S15 = _S13 / _S14.xy;

#line 198
    float2 dir_0 = float2(_S15) * 2.0 - 1.0;
    uint3 _S16 = DispatchRaysDimensions();

#line 199
    uint _S17 = _S16.x;

#line 199
    uint3 _S18 = DispatchRaysDimensions();

#line 199
    uint _S19 = _S17 / _S18.y;

#line 199
    float aspectRatio_0 = float(_S19);

    RayDesc rayDesc_1;
    rayDesc_1.Origin = float3(0.0, 0.0, 0.0);
    rayDesc_1.Direction = normalize(float3(dir_0.x * aspectRatio_0, - dir_0.y, 1.0));
    rayDesc_1.TMin = 0.00999999977648258209;
    rayDesc_1.TMax = 10000.0;

    RayPayload_0 payload_2;
    payload_2.RayHitT_0 = 10000.0;



    float _S20 = CheckTraceRay_0(payload_2, rayDesc_1);

    float _S21 = CheckSysValueIntrinsics_0();

    float _S22 = attribs_0.barycentrics.x;

#line 216
    float _S23 = attribs_0.barycentrics.y;
    float val_25 = _S20 + _S21 + (1.0 - _S22 - _S23) + _S22 + _S23;

    CallableParams_0 params_0;
    CallShader(0U, params_0);


    payload_2.RayHitT_0 = val_25 + params_0.value_0;
    return;
}

