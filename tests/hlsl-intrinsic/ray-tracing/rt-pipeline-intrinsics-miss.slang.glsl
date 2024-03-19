#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_NV_ray_tracing_motion_blur : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 6 0
layout(binding = 0)
uniform accelerationStructureEXT accelStruct_0;


#line 13
struct CallableParams_0
{
    float value_0;
};


#line 9611 1
layout(location = 0)
callableDataEXT
CallableParams_0 p_0;


#line 8 0
struct RayPayload_0
{
    float RayHitT_0;
};


#line 9764 1
layout(location = 0)
rayPayloadEXT
RayPayload_0 p_1;


#line 9676
layout(location = 1)
rayPayloadEXT
RayPayload_0 p_2;


#line 9533
struct RayDesc_0
{
    vec3 Origin_0;
    float TMin_0;
    vec3 Direction_0;
    float TMax_0;
};


#line 9659
void TraceRay_0(accelerationStructureEXT AccelerationStructure_0, uint RayFlags_0, uint InstanceInclusionMask_0, uint RayContributionToHitGroupIndex_0, uint MultiplierForGeometryContributionToHitGroupIndex_0, uint MissShaderIndex_0, RayDesc_0 Ray_0, inout RayPayload_0 Payload_0)
{

#line 9678
    p_2 = Payload_0;
    traceRayEXT(AccelerationStructure_0, RayFlags_0, InstanceInclusionMask_0, RayContributionToHitGroupIndex_0, MultiplierForGeometryContributionToHitGroupIndex_0, MissShaderIndex_0, Ray_0.Origin_0, Ray_0.TMin_0, Ray_0.Direction_0, Ray_0.TMax_0, 1);

#line 9691
    Payload_0 = p_2;

#line 9720
    return;
}


#line 9747
void TraceMotionRay_0(accelerationStructureEXT AccelerationStructure_1, uint RayFlags_1, uint InstanceInclusionMask_1, uint RayContributionToHitGroupIndex_1, uint MultiplierForGeometryContributionToHitGroupIndex_1, uint MissShaderIndex_1, RayDesc_0 Ray_1, float CurrentTime_0, inout RayPayload_0 Payload_1)
{

#line 9766
    p_1 = Payload_1;
    traceRayMotionNV(AccelerationStructure_1, RayFlags_1, InstanceInclusionMask_1, RayContributionToHitGroupIndex_1, MultiplierForGeometryContributionToHitGroupIndex_1, MissShaderIndex_1, Ray_1.Origin_0, Ray_1.TMin_0, Ray_1.Direction_0, Ray_1.TMax_0, CurrentTime_0, 0);

#line 9780
    Payload_1 = p_1;

#line 9814
    return;
}


#line 74 0
float CheckTraceRay_0(RayPayload_0 payload_0, RayDesc_0 rayDesc_0)
{

#line 74
    RayPayload_0 _S1 = payload_0;

#line 79
    TraceRay_0(accelStruct_0, 0U, 4294967295U, 0U, 1U, 0U, rayDesc_0, _S1);
    float _S2 = _S1.RayHitT_0;
    TraceRay_0(accelStruct_0, 1U, 4294967295U, 0U, 1U, 0U, rayDesc_0, _S1);
    float val_0 = _S2 + _S1.RayHitT_0;
    TraceRay_0(accelStruct_0, 2U, 4294967295U, 0U, 1U, 0U, rayDesc_0, _S1);
    float val_1 = val_0 + _S1.RayHitT_0;
    TraceRay_0(accelStruct_0, 4U, 4294967295U, 0U, 1U, 0U, rayDesc_0, _S1);
    float val_2 = val_1 + _S1.RayHitT_0;
    TraceRay_0(accelStruct_0, 8U, 4294967295U, 0U, 1U, 0U, rayDesc_0, _S1);
    float val_3 = val_2 + _S1.RayHitT_0;
    TraceRay_0(accelStruct_0, 16U, 4294967295U, 0U, 1U, 0U, rayDesc_0, _S1);
    float val_4 = val_3 + _S1.RayHitT_0;
    TraceRay_0(accelStruct_0, 32U, 4294967295U, 0U, 1U, 0U, rayDesc_0, _S1);
    float val_5 = val_4 + _S1.RayHitT_0;
    TraceRay_0(accelStruct_0, 64U, 4294967295U, 0U, 1U, 0U, rayDesc_0, _S1);
    float val_6 = val_5 + _S1.RayHitT_0;
    TraceRay_0(accelStruct_0, 128U, 4294967295U, 0U, 1U, 0U, rayDesc_0, _S1);
    float val_7 = val_6 + _S1.RayHitT_0;
    TraceRay_0(accelStruct_0, 256U, 4294967295U, 0U, 1U, 0U, rayDesc_0, _S1);
    float val_8 = val_7 + _S1.RayHitT_0;
    TraceRay_0(accelStruct_0, 512U, 4294967295U, 0U, 1U, 0U, rayDesc_0, _S1);
    float val_9 = val_8 + _S1.RayHitT_0;


    TraceMotionRay_0(accelStruct_0, 0U, 4294967295U, 0U, 1U, 0U, rayDesc_0, 1.0, _S1);
    float val_10 = val_9 + _S1.RayHitT_0;
    TraceMotionRay_0(accelStruct_0, 1U, 4294967295U, 0U, 1U, 0U, rayDesc_0, 1.0, _S1);
    float val_11 = val_10 + _S1.RayHitT_0;
    TraceMotionRay_0(accelStruct_0, 2U, 4294967295U, 0U, 1U, 0U, rayDesc_0, 1.0, _S1);
    float val_12 = val_11 + _S1.RayHitT_0;
    TraceMotionRay_0(accelStruct_0, 4U, 4294967295U, 0U, 1U, 0U, rayDesc_0, 1.0, _S1);
    float val_13 = val_12 + _S1.RayHitT_0;
    TraceMotionRay_0(accelStruct_0, 8U, 4294967295U, 0U, 1U, 0U, rayDesc_0, 1.0, _S1);
    float val_14 = val_13 + _S1.RayHitT_0;
    TraceMotionRay_0(accelStruct_0, 16U, 4294967295U, 0U, 1U, 0U, rayDesc_0, 1.0, _S1);
    float val_15 = val_14 + _S1.RayHitT_0;
    TraceMotionRay_0(accelStruct_0, 32U, 4294967295U, 0U, 1U, 0U, rayDesc_0, 1.0, _S1);
    float val_16 = val_15 + _S1.RayHitT_0;
    TraceMotionRay_0(accelStruct_0, 64U, 4294967295U, 0U, 1U, 0U, rayDesc_0, 1.0, _S1);
    float val_17 = val_16 + _S1.RayHitT_0;
    TraceMotionRay_0(accelStruct_0, 128U, 4294967295U, 0U, 1U, 0U, rayDesc_0, 1.0, _S1);
    float val_18 = val_17 + _S1.RayHitT_0;
    TraceMotionRay_0(accelStruct_0, 256U, 4294967295U, 0U, 1U, 0U, rayDesc_0, 1.0, _S1);
    float val_19 = val_18 + _S1.RayHitT_0;
    TraceMotionRay_0(accelStruct_0, 512U, 4294967295U, 0U, 1U, 0U, rayDesc_0, 1.0, _S1);



    return val_19 + _S1.RayHitT_0;
}


#line 21
float CheckRayDispatchValues_0()
{


    uvec3 ri_0 = ((gl_LaunchIDEXT));
    uvec3 rd_0 = ((gl_LaunchSizeEXT));

#line 31
    return float(ri_0.x) + float(ri_0.y) + float(ri_0.z) + float(rd_0.x) + float(rd_0.y) + float(rd_0.z);
}

float CheckRaySystemValues_0()
{


    vec3 wro_0 = ((gl_WorldRayOriginEXT));
    float val_20 = wro_0.x + wro_0.y + wro_0.z;

    vec3 wrd_0 = ((gl_WorldRayDirectionEXT));
    float val_21 = val_20 + wrd_0.x + wrd_0.y + wrd_0.z;

    float rayTMin_0 = ((gl_RayTminEXT));
    float val_22 = val_21 + rayTMin_0;

    float rayTCurrent_0 = ((gl_RayTmaxEXT));
    float val_23 = val_22 + rayTCurrent_0;

    uint rayFlags_0 = ((gl_IncomingRayFlagsEXT));

#line 50
    float val_24;
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
            val_24 = val_23 + 1.0;

#line 51
            break;
        }
    default:
        {

#line 51
            val_24 = val_23;

#line 51
            break;
        }
    }

#line 71
    return val_24;
}


#line 130
float CheckSysValueIntrinsics_0()
{


    float _S3 = CheckRayDispatchValues_0();
    float _S4 = CheckRaySystemValues_0();

    return _S3 + _S4;
}


#line 9603 1
void CallShader_0(uint shaderIndex_0, inout CallableParams_0 payload_1)
{

#line 9613
    p_0 = payload_1;
    executeCallableEXT(shaderIndex_0, 0);
    payload_1 = p_0;

#line 9629
    return;
}


#line 9629
rayPayloadInEXT RayPayload_0 _S5;


#line 141 0
void main()
{
    uvec3 _S6 = ((gl_LaunchIDEXT));

#line 143
    uvec2 _S7 = _S6.xy;

#line 143
    uvec3 _S8 = ((gl_LaunchSizeEXT));

#line 143
    uvec2 _S9 = _S7 / _S8.xy;

#line 143
    vec2 dir_0 = vec2(_S9) * 2.0 - 1.0;
    uvec3 _S10 = ((gl_LaunchSizeEXT));

#line 144
    uint _S11 = _S10.x;

#line 144
    uvec3 _S12 = ((gl_LaunchSizeEXT));

#line 144
    uint _S13 = _S11 / _S12.y;

#line 144
    float aspectRatio_0 = float(_S13);

    RayDesc_0 rayDesc_1;
    rayDesc_1.Origin_0 = vec3(0.0, 0.0, 0.0);
    rayDesc_1.Direction_0 = normalize(vec3(dir_0.x * aspectRatio_0, - dir_0.y, 1.0));
    rayDesc_1.TMin_0 = 0.00999999977648258209;
    rayDesc_1.TMax_0 = 10000.0;

    RayPayload_0 payload_2;
    payload_2.RayHitT_0 = 10000.0;



    float _S14 = CheckTraceRay_0(payload_2, rayDesc_1);

    float _S15 = CheckSysValueIntrinsics_0();

#line 159
    float val_25 = _S14 + _S15;

    CallableParams_0 params_0;
    CallShader_0(0U, params_0);


    payload_2.RayHitT_0 = val_25 + params_0.value_0;
    return;
}

