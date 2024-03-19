#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif
#pragma warning(disable: 3557)


#line 6 "D:/slang/tests/hlsl-intrinsic/ray-tracing/rt-pipeline-intrinsics-rgen.slang"
RaytracingAccelerationStructure accelStruct_0 : register(t0);


RWTexture2D<float4 > screenOutput_0 : register(u0);

struct RayPayload_0
{
    float RayHitT_0;
};


#line 37
float CheckTraceRay_0(RayPayload_0 payload_0, RayDesc rayDesc_0)
{

#line 37
    RayPayload_0 _S1 = payload_0;

#line 42
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

#line 90
    return val_8 + _S1.RayHitT_0;
}


#line 24
float CheckRayDispatchValues_0()
{


    uint3 ri_0 = DispatchRaysIndex();
    uint3 rd_0 = DispatchRaysDimensions();

#line 34
    return float(ri_0.x) + float(ri_0.y) + float(ri_0.z) + float(rd_0.x) + float(rd_0.y) + float(rd_0.z);
}


#line 93
float CheckSysValueIntrinsics_0()
{


    float _S3 = CheckRayDispatchValues_0();

    return _S3;
}


#line 16
struct CallableParams_0
{
    float value_0;
};


#line 103
[shader("raygeneration")]void main()
{
    uint3 _S4 = DispatchRaysIndex();

#line 105
    uint2 _S5 = _S4.xy;

#line 105
    uint3 _S6 = DispatchRaysDimensions();

#line 105
    uint2 _S7 = _S5 / _S6.xy;

#line 105
    float2 dir_0 = float2(_S7) * 2.0 - 1.0;
    uint3 _S8 = DispatchRaysDimensions();

#line 106
    uint _S9 = _S8.x;

#line 106
    uint3 _S10 = DispatchRaysDimensions();

#line 106
    uint _S11 = _S9 / _S10.y;

#line 106
    float aspectRatio_0 = float(_S11);

    RayDesc rayDesc_1;
    rayDesc_1.Origin = float3(0.0, 0.0, 0.0);
    rayDesc_1.Direction = normalize(float3(dir_0.x * aspectRatio_0, - dir_0.y, 1.0));
    rayDesc_1.TMin = 0.00999999977648258209;
    rayDesc_1.TMax = 10000.0;

    RayPayload_0 payload_1;
    payload_1.RayHitT_0 = 10000.0;



    float _S12 = CheckTraceRay_0(payload_1, rayDesc_1);

    if(_S12 < 10000.0)
    {
        float _S13 = CheckSysValueIntrinsics_0();

#line 123
        float val_9 = _S12 + _S13;
        uint3 _S14 = DispatchRaysIndex();

#line 124
        screenOutput_0[_S14.xy] = float4(val_9, val_9, val_9, 1.0);

#line 121
    }
    else
    {

#line 128
        CallableParams_0 params_0;
        CallShader(0U, params_0);

        uint3 _S15 = DispatchRaysIndex();

#line 131
        screenOutput_0[_S15.xy] = float4(params_0.value_0, params_0.value_0, params_0.value_0, params_0.value_0);

#line 121
    }

#line 133
    return;
}

