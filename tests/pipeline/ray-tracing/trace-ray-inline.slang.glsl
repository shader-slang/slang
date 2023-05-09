#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_ray_query : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 91 "tests/pipeline/ray-tracing/trace-ray-inline.slang"
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


#line 91
layout(binding = 2)
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


#line 86
layout(std430, binding = 1) buffer _S2 {
    int _data[];
} resultBuffer_0;

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


#line 105
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{

#line 107
    uint index_0 = gl_GlobalInvocationID.x;

#line 112
    MyRayPayload_0 payload_5;

#line 112
    payload_5.value_1 = -1;

#line 109
    rayQueryEXT query_0;

#line 114
    rayQueryInitializeEXT((query_0), (myAccelerationStructure_0), (C_0.rayFlags_0 | 512), (C_0.instanceMask_0), (C_0.origin_0), (C_0.tMin_0), (C_0.direction_0), (C_0.tMax_0));

#line 114
    MyProceduralHitAttrs_0 committedProceduralAttrs_0;

#line 114
    for(;;)
    {

#line 123
        bool _S3 = rayQueryProceedEXT(query_0);

#line 123
        if(!_S3)
        {

#line 123
            break;
        }

#line 123
        MyProceduralHitAttrs_0 committedProceduralAttrs_1;

        switch((rayQueryGetIntersectionTypeEXT((query_0), false)))
        {
        case 1U:
            {
                MyProceduralHitAttrs_0 candidateProceduralAttrs_0;

#line 129
                candidateProceduralAttrs_0.value_0 = 0;
                float tHit_1 = 0.0;
                bool _S4 = myProceduralIntersection_0(tHit_1, candidateProceduralAttrs_0);

#line 131
                if(_S4)
                {
                    bool _S5 = myProceduralAnyHit_0(payload_5);

#line 133
                    if(_S5)
                    {
                        rayQueryGenerateIntersectionEXT(query_0, tHit_1);
                        MyProceduralHitAttrs_0 _S6 = candidateProceduralAttrs_0;
                        if(C_0.shouldStopAtFirstHit_0 != 0U)
                        {

#line 138
                            rayQueryTerminateEXT(query_0);

#line 137
                        }
                        else
                        {

#line 137
                        }

#line 137
                        committedProceduralAttrs_1 = _S6;

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
                bool _S7 = myTriangleAnyHit_0(payload_5);

#line 146
                if(_S7)
                {
                    rayQueryConfirmIntersectionEXT(query_0);
                    if(C_0.shouldStopAtFirstHit_0 != 0U)
                    {

#line 150
                        rayQueryTerminateEXT(query_0);

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
    switch((rayQueryGetIntersectionTypeEXT((query_0), true)))
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
    ((resultBuffer_0)._data[(index_0)]) = payload_5.value_1;
    return;
}

