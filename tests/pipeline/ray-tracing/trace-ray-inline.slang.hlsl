#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif
#pragma warning(disable: 3557)


#line 91 "tests/pipeline/ray-tracing/trace-ray-inline.slang"
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


#line 91
cbuffer C_0 : register(b0)
{
    SLANG_ParameterGroup_C_0 C_0;
}

#line 12
RaytracingAccelerationStructure myAccelerationStructure_0 : register(t0);


#line 86
RWStructuredBuffer<int > resultBuffer_0 : register(u0);


#line 59
struct MyProceduralHitAttrs_0
{
    int value_0;
};


#line 81
bool myProceduralIntersection_0(inout float tHit_0, inout MyProceduralHitAttrs_0 hitAttrs_0)
{
    return true;
}


#line 26
struct MyRayPayload_0
{
    int value_1;
};


#line 69
bool myProceduralAnyHit_0(inout MyRayPayload_0 payload_0)
{
    return true;
}


#line 51
bool myTriangleAnyHit_0(inout MyRayPayload_0 payload_1)
{
    return true;
}


#line 40
void myTriangleClosestHit_0(inout MyRayPayload_0 payload_2)
{
    payload_2.value_1 = int(1);
    return;
}


#line 65
void myProceduralClosestHit_0(inout MyRayPayload_0 payload_3, MyProceduralHitAttrs_0 attrs_0)
{
    payload_3.value_1 = attrs_0.value_0;
    return;
}


#line 33
void myMiss_0(inout MyRayPayload_0 payload_4)
{
    payload_4.value_1 = int(0);
    return;
}


#line 105
[shader("compute")][numthreads(1, 1, 1)]
void main(uint3 tid_0 : SV_DISPATCHTHREADID)
{

#line 107
    uint index_0 = tid_0.x;

#line 112
    MyRayPayload_0 payload_5;

#line 112
    payload_5.value_1 = int(-1);
    RayDesc ray_0 = { C_0.origin_0, C_0.tMin_0, C_0.direction_0, C_0.tMax_0 };

#line 109
    RayQuery<int(512) > query_0;

#line 114
    query_0.TraceRayInline(myAccelerationStructure_0, C_0.rayFlags_0, C_0.instanceMask_0, ray_0);

#line 114
    MyProceduralHitAttrs_0 committedProceduralAttrs_0;

#line 114
    for(;;)
    {

#line 123
        bool _S1 = query_0.Proceed();

#line 123
        if(!_S1)
        {

#line 123
            break;
        }

#line 123
        MyProceduralHitAttrs_0 committedProceduralAttrs_1;

        switch(query_0.CandidateType())
        {
        case 1U:
            {
                MyProceduralHitAttrs_0 candidateProceduralAttrs_0;

#line 129
                candidateProceduralAttrs_0.value_0 = int(0);
                float tHit_1 = 0.0;
                bool _S2 = myProceduralIntersection_0(tHit_1, candidateProceduralAttrs_0);

#line 131
                if(_S2)
                {
                    bool _S3 = myProceduralAnyHit_0(payload_5);

#line 133
                    if(_S3)
                    {
                        query_0.CommitProceduralPrimitiveHit(tHit_1);
                        MyProceduralHitAttrs_0 _S4 = candidateProceduralAttrs_0;
                        if(C_0.shouldStopAtFirstHit_0 != 0U)
                        {

#line 138
                            query_0.Abort();

#line 137
                        }
                        else
                        {

#line 137
                        }

#line 137
                        committedProceduralAttrs_1 = _S4;

#line 137
                    }
                    else
                    {

#line 137
                        committedProceduralAttrs_1 = committedProceduralAttrs_0;

#line 137
                    }

#line 137
                }
                else
                {

#line 137
                    committedProceduralAttrs_1 = committedProceduralAttrs_0;

#line 137
                }

#line 137
                break;
            }
        case 0U:
            {

#line 146
                bool _S5 = myTriangleAnyHit_0(payload_5);

#line 146
                if(_S5)
                {
                    query_0.CommitNonOpaqueTriangleHit();
                    if(C_0.shouldStopAtFirstHit_0 != 0U)
                    {

#line 150
                        query_0.Abort();

#line 149
                    }
                    else
                    {

#line 149
                    }

#line 146
                }
                else
                {

#line 146
                }

#line 146
                committedProceduralAttrs_1 = committedProceduralAttrs_0;

#line 146
                break;
            }
        default:
            {

#line 146
                committedProceduralAttrs_1 = committedProceduralAttrs_0;

#line 146
                break;
            }
        }

#line 121
        committedProceduralAttrs_0 = committedProceduralAttrs_1;

#line 121
    }

#line 160
    switch(query_0.CommittedStatus())
    {
    case 1U:
        {

#line 163
            myTriangleClosestHit_0(payload_5);
            break;
        }
    case 2U:
        {

#line 167
            myProceduralClosestHit_0(payload_5, committedProceduralAttrs_0);
            break;
        }
    case 0U:
        {

#line 171
            myMiss_0(payload_5);
            break;
        }
    default:
        {

#line 172
            break;
        }
    }
    resultBuffer_0[index_0] = payload_5.value_1;
    return;
}

