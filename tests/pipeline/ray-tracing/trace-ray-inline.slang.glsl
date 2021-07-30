// trace-ray-inline.slang.glsl
//TEST_IGNORE_FILE:

#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_ray_query : require

struct SLANG_ParameterGroup_C_0
{
    vec3 origin_0;
    float tMin_0;
    vec3 direction_0;
    float tMax_0;
    uint rayFlags_0;
    uint instanceMask_0;
    uint shouldStopAtFirstHit_0;
};

layout(binding = 1)
layout(std140) uniform _S1
{
    SLANG_ParameterGroup_C_0 _data;
} C_0;

struct RayDesc_0
{
    vec3 Origin_0;
    float TMin_0;
    vec3 Direction_0;
    float TMax_0;
};

void RayQuery_TraceRayInline_0(rayQueryEXT this_0, accelerationStructureEXT accelerationStructure_0, uint rayFlags_1, uint instanceInclusionMask_0, RayDesc_0 ray_0)
{
    rayQueryInitializeEXT((this_0), (accelerationStructure_0), (rayFlags_1), (instanceInclusionMask_0), (ray_0.Origin_0), (ray_0.TMin_0), (ray_0.Direction_0), (ray_0.TMax_0));
    return;
}

layout(binding = 0)
uniform accelerationStructureEXT myAccelerationStructure_0;

struct MyProceduralHitAttrs_0
{
    int value_0;
};

bool myProceduralIntersection_0(inout float tHit_0, inout MyProceduralHitAttrs_0 hitAttrs_0)
{
    return true;
}

struct MyRayPayload_0
{
    int value_1;
};

bool myProceduralAnyHit_0(inout MyRayPayload_0 payload_0)
{
    return true;
}

bool myTriangleAnyHit_0(inout MyRayPayload_0 payload_1)
{
    return true;
}

void myTriangleClosestHit_0(inout MyRayPayload_0 payload_2)
{
    payload_2.value_1 = 1;
    return;
}

void myProceduralClosestHit_0(inout MyRayPayload_0 payload_3, MyProceduralHitAttrs_0 attrs_0)
{
    payload_3.value_1 = attrs_0.value_0;
    return;
}

void myMiss_0(inout MyRayPayload_0 payload_4)
{
    payload_4.value_1 = 0;
    return;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    MyProceduralHitAttrs_0 committedProceduralAttrs_0;
    MyProceduralHitAttrs_0 committedProceduralAttrs_1;
    MyProceduralHitAttrs_0 committedProceduralAttrs_2;
    MyProceduralHitAttrs_0 committedProceduralAttrs_3;

    rayQueryEXT query_0;

    MyRayPayload_0 payload_5;
    MyRayPayload_0 _S2 = { -1 };
    payload_5 = _S2;
 
    RayDesc_0 ray_1 = { C_0._data.origin_0, C_0._data.tMin_0, C_0._data.direction_0, C_0._data.tMax_0 };
    RayQuery_TraceRayInline_0(query_0, myAccelerationStructure_0, C_0._data.rayFlags_0, C_0._data.instanceMask_0, ray_1);

    MyProceduralHitAttrs_0 _S3;
    committedProceduralAttrs_0 = _S3;

    for(;;)
    {
        bool _S4 = rayQueryProceedEXT(query_0);
        if(!_S4)
        {
            break;
        }
        uint _S5 = (rayQueryGetIntersectionTypeEXT((query_0), false));
        switch(_S5)
        {
        case uint(1):
            {
                MyProceduralHitAttrs_0 candidateProceduralAttrs_0;
                MyProceduralHitAttrs_0 _S6 = { 0 };
                candidateProceduralAttrs_0 = _S6;

                float tHit_1;
                tHit_1 = 0.00000000000000000000;

                bool _S7 = myProceduralIntersection_0(tHit_1, candidateProceduralAttrs_0);
                if(_S7)
                {
                    bool _S8 = myProceduralAnyHit_0(payload_5);
                    if(_S8)
                    {
                        rayQueryGenerateIntersectionEXT(query_0, tHit_1);
                        MyProceduralHitAttrs_0 _S9 = candidateProceduralAttrs_0;
                        if(bool(C_0._data.shouldStopAtFirstHit_0))
                        {
                            rayQueryTerminateEXT(query_0);
                        }
                        else
                        {
                        }
                        committedProceduralAttrs_1 = _S9;
                    }
                    else
                    {
                        committedProceduralAttrs_1 = committedProceduralAttrs_0;
                    }
                    committedProceduralAttrs_2 = committedProceduralAttrs_1;
                }
                else
                {
                    committedProceduralAttrs_2 = committedProceduralAttrs_0;
                }
                committedProceduralAttrs_3 = committedProceduralAttrs_2;
                break;
            }
        case uint(0):
            {
                bool _S10 = myTriangleAnyHit_0(payload_5);
                if(_S10)
                {
                    rayQueryConfirmIntersectionEXT(query_0);
                    if(bool(C_0._data.shouldStopAtFirstHit_0))
                    {
                        rayQueryTerminateEXT(query_0);
                    }
                    else
                    {
                    }
                }
                else
                {
                }
                committedProceduralAttrs_3 = committedProceduralAttrs_0;
                break;
            }
        default:
            {
                committedProceduralAttrs_3 = committedProceduralAttrs_0;
                break;
            }
        }
        committedProceduralAttrs_0 = committedProceduralAttrs_3;
    }
    uint _S11 = (rayQueryGetIntersectionTypeEXT((query_0), true));
    switch(_S11)
    {
    case uint(1):
        {
            myTriangleClosestHit_0(payload_5);
            break;
        }
    case uint(2):
        {
            myProceduralClosestHit_0(payload_5, committedProceduralAttrs_0);
            break;
        }
    case uint(0):
        {
            myMiss_0(payload_5);
            break;
        }
    default:
        {
            break;
        }
    }
    return;
}
