#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif
#pragma warning(disable: 3557)

struct SLANG_ParameterGroup_C_0
{
    float3 origin_0;
    float tMin_0;
    float3 direction_0;
    float tMax_0;
    uint rayFlags_0;
    uint instanceMask_0;
    uint shouldStopAtFirstHit_0;
};

cbuffer C_0 : register(b0)
{
    SLANG_ParameterGroup_C_0 C_0;
}
RaytracingAccelerationStructure myAccelerationStructure_0 : register(t0);

RWStructuredBuffer<int > resultBuffer_0 : register(u0);

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
    payload_2.value_1 = int(1);
    return;
}

void myProceduralClosestHit_0(inout MyRayPayload_0 payload_3, MyProceduralHitAttrs_0 attrs_0)
{
    payload_3.value_1 = attrs_0.value_0;
    return;
}

void myMiss_0(inout MyRayPayload_0 payload_4)
{
    payload_4.value_1 = int(0);
    return;
}

[shader("compute")][numthreads(1, 1, 1)]
void main(uint3 tid_0 : SV_DISPATCHTHREADID)
{
    uint index_0 = tid_0.x;
    MyRayPayload_0 payload_5;
    payload_5.value_1 = int(-1);
    RayDesc ray_0 = { C_0.origin_0, C_0.tMin_0, C_0.direction_0, C_0.tMax_0 };
    RayQuery<512U > query_0;
    query_0.TraceRayInline(myAccelerationStructure_0, C_0.rayFlags_0, C_0.instanceMask_0, ray_0);
    MyProceduralHitAttrs_0 committedProceduralAttrs_0;
    for(;;)
    {
        bool _S1 = query_0.Proceed();
        if(!_S1)
        {
            break;
        }
        uint _S2 = query_0.CandidateType();
        MyProceduralHitAttrs_0 committedProceduralAttrs_1;
        switch(_S2)
        {
        case 1U:
            {
                MyProceduralHitAttrs_0 candidateProceduralAttrs_0;
                candidateProceduralAttrs_0.value_0 = int(0);
                float tHit_1 = 0.0;
                bool _S3 = myProceduralIntersection_0(tHit_1, candidateProceduralAttrs_0);
                if(_S3)
                {
                    bool _S4 = myProceduralAnyHit_0(payload_5);
                    if(_S4)
                    {
                        query_0.CommitProceduralPrimitiveHit(tHit_1);
                        MyProceduralHitAttrs_0 _S5 = candidateProceduralAttrs_0;
                        if(C_0.shouldStopAtFirstHit_0 != 0U)
                        {
                            query_0.Abort();
                        }
                        committedProceduralAttrs_1 = _S5;
                    }
                    else
                    {
                        committedProceduralAttrs_1 = committedProceduralAttrs_0;
                    }
                }
                else
                {
                    committedProceduralAttrs_1 = committedProceduralAttrs_0;
                }
                break;
            }
        case 0U:
            {
                bool _S6 = myTriangleAnyHit_0(payload_5);
                if(_S6)
                {
                    query_0.CommitNonOpaqueTriangleHit();
                    if(C_0.shouldStopAtFirstHit_0 != 0U)
                    {
                        query_0.Abort();
                    }
                }
                committedProceduralAttrs_1 = committedProceduralAttrs_0;
                break;
            }
        default:
            {
                committedProceduralAttrs_1 = committedProceduralAttrs_0;
                break;
            }
        }
        committedProceduralAttrs_0 = committedProceduralAttrs_1;
    }
    uint _S7 = query_0.CommittedStatus();
    switch(_S7)
    {
    case 1U:
        {
            myTriangleClosestHit_0(payload_5);
            break;
        }
    case 2U:
        {
            myProceduralClosestHit_0(payload_5, committedProceduralAttrs_0);
            break;
        }
    case 0U:
        {
            myMiss_0(payload_5);
            break;
        }
    default:
        {
            break;
        }
    }
    resultBuffer_0[index_0] = payload_5.value_1;
    return;
}
