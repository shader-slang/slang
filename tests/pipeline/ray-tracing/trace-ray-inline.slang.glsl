#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_ray_query : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 89 "tests/pipeline/ray-tracing/trace-ray-inline.slang"
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


#line 89
layout(binding = 1)
layout(std140) uniform _S1
{
    vec3 origin_0;
    float tMin_0;
    vec3 direction_0;
    float tMax_0;
    uint rayFlags_0;
    uint instanceMask_0;
    uint shouldStopAtFirstHit_0;
}C_0;

#line 12
layout(binding = 0)
uniform accelerationStructureEXT myAccelerationStructure_0;


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
    payload_2.value_1 = 1;
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
    payload_4.value_1 = 0;
    return;
}


#line 103
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{

    rayQueryEXT query_0;


    MyRayPayload_0 payload_5;

#line 110
    payload_5.value_1 = -1;

    rayQueryInitializeEXT((query_0), (myAccelerationStructure_0), (C_0.rayFlags_0 | 512), (C_0.instanceMask_0), (C_0.origin_0), (C_0.tMin_0), (C_0.direction_0), (C_0.tMax_0));

#line 112
    MyProceduralHitAttrs_0 committedProceduralAttrs_0;

#line 112
    for(;;)
    {

#line 121
        bool _S2 = rayQueryProceedEXT(query_0);

#line 121
        if(!_S2)
        {

#line 121
            break;
        }
        uint _S3 = (rayQueryGetIntersectionTypeEXT((query_0), false));

#line 123
        MyProceduralHitAttrs_0 committedProceduralAttrs_1;

#line 123
        switch(_S3)
        {
        case 1U:
            {
                MyProceduralHitAttrs_0 candidateProceduralAttrs_0;

#line 127
                candidateProceduralAttrs_0.value_0 = 0;
                float tHit_1 = 0.0;
                bool _S4 = myProceduralIntersection_0(tHit_1, candidateProceduralAttrs_0);

#line 129
                if(_S4)
                {
                    bool _S5 = myProceduralAnyHit_0(payload_5);

#line 131
                    if(_S5)
                    {
                        rayQueryGenerateIntersectionEXT(query_0, tHit_1);
                        MyProceduralHitAttrs_0 _S6 = candidateProceduralAttrs_0;
                        if(C_0.shouldStopAtFirstHit_0 != 0U)
                        {

#line 136
                            rayQueryTerminateEXT(query_0);

#line 135
                        }
                        else
                        {

#line 135
                        }

#line 135
                        committedProceduralAttrs_1 = _S6;

#line 135
                    }
                    else
                    {

#line 135
                        committedProceduralAttrs_1 = committedProceduralAttrs_0;

#line 135
                    }

#line 135
                }
                else
                {

#line 135
                    committedProceduralAttrs_1 = committedProceduralAttrs_0;

#line 135
                }

#line 135
                break;
            }
        case 0U:
            {

#line 144
                bool _S7 = myTriangleAnyHit_0(payload_5);

#line 144
                if(_S7)
                {
                    rayQueryConfirmIntersectionEXT(query_0);
                    if(C_0.shouldStopAtFirstHit_0 != 0U)
                    {

#line 148
                        rayQueryTerminateEXT(query_0);

#line 147
                    }
                    else
                    {

#line 147
                    }

#line 144
                }
                else
                {

#line 144
                }

#line 144
                committedProceduralAttrs_1 = committedProceduralAttrs_0;

#line 144
                break;
            }
        default:
            {

#line 144
                committedProceduralAttrs_1 = committedProceduralAttrs_0;

#line 144
                break;
            }
        }

#line 119
        committedProceduralAttrs_0 = committedProceduralAttrs_1;

#line 119
    }

#line 158
    uint _S8 = (rayQueryGetIntersectionTypeEXT((query_0), true));

#line 158
    switch(_S8)
    {
    case 1U:
        {

#line 161
            myTriangleClosestHit_0(payload_5);
            break;
        }
    case 2U:
        {

#line 165
            myProceduralClosestHit_0(payload_5, committedProceduralAttrs_0);
            break;
        }
    case 0U:
        {

#line 169
            myMiss_0(payload_5);
            break;
        }
    default:
        {

#line 170
            break;
        }
    }

#line 172
    return;
}

