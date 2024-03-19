#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif
#pragma warning(disable: 3557)


#line 6 "D:/slang/tests/hlsl-intrinsic/ray-tracing/rt-pipeline-intrinsics-miss.slang"
RaytracingAccelerationStructure accelStruct_0 : register(t0);

struct RayPayload_0
{
    float RayHitT_0;
};


#line 74
float CheckTraceRay_0(RayPayload_0 payload_0, RayDesc rayDesc_0)
{

#line 74
    RayPayload_0 _S1 = payload_0;

#line 79
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

#line 127
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


#line 130
float CheckSysValueIntrinsics_0()
{


    float _S3 = CheckRayDispatchValues_0();
    float _S4 = CheckRaySystemValues_0();

    return _S3 + _S4;
}


#line 13
struct CallableParams_0
{
    float value_0;
};


#line 141
[shader("miss")]void main(inout RayPayload_0 payload_1)
{
    uint3 _S5 = DispatchRaysIndex();

#line 143
    uint2 _S6 = _S5.xy;

#line 143
    uint3 _S7 = DispatchRaysDimensions();

#line 143
    uint2 _S8 = _S6 / _S7.xy;

#line 143
    float2 dir_0 = float2(_S8) * 2.0 - 1.0;
    uint3 _S9 = DispatchRaysDimensions();

#line 144
    uint _S10 = _S9.x;

#line 144
    uint3 _S11 = DispatchRaysDimensions();

#line 144
    uint _S12 = _S10 / _S11.y;

#line 144
    float aspectRatio_0 = float(_S12);

    RayDesc rayDesc_1;
    rayDesc_1.Origin = float3(0.0, 0.0, 0.0);
    rayDesc_1.Direction = normalize(float3(dir_0.x * aspectRatio_0, - dir_0.y, 1.0));
    rayDesc_1.TMin = 0.00999999977648258209;
    rayDesc_1.TMax = 10000.0;

    RayPayload_0 payload_2;
    payload_2.RayHitT_0 = 10000.0;



    float _S13 = CheckTraceRay_0(payload_2, rayDesc_1);

    float _S14 = CheckSysValueIntrinsics_0();

#line 159
    float val_14 = _S13 + _S14;

    CallableParams_0 params_0;
    CallShader(0U, params_0);


    payload_2.RayHitT_0 = val_14 + params_0.value_0;
    return;
}

