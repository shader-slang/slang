// trace-ray-inline.slang.hlsl
//TEST_IGNORE_FILE:

struct SLANG_ParameterGroup_C_0
{
    vector<float,3> origin_0;
    float tMin_0;
    vector<float,3> direction_0;
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


[shader("compute")]
[numthreads(1, 1, 1)]
void main(vector<uint,3> tid_0 : SV_DISPATCHTHREADID)
{
    MyRayPayload_0 payload_5;
    MyProceduralHitAttrs_0 committedProceduralAttrs_0;
    MyProceduralHitAttrs_0 committedProceduralAttrs_1;
    MyRayPayload_0 payload_6;
    MyProceduralHitAttrs_0 committedProceduralAttrs_2;
    MyRayPayload_0 payload_7;
    MyProceduralHitAttrs_0 committedProceduralAttrs_3;

    RayQuery<int(512) > query_0;

    MyRayPayload_0 _S1 = { int(-1) };
    RayDesc ray_0 = { C_0.origin_0, C_0.tMin_0, C_0.direction_0, C_0.tMax_0 };
    query_0.TraceRayInline(myAccelerationStructure_0, C_0.rayFlags_0, C_0.instanceMask_0, ray_0);

    MyProceduralHitAttrs_0 _S2;

    payload_5 = _S1;
    committedProceduralAttrs_0 = _S2;
    for(;;)
    {
        bool _S3 = query_0.Proceed();

        if(!_S3)
        {
            break;
        }
        uint _S4 = query_0.CandidateType();

        switch(_S4)
        {
        case (uint) int(1):
            {
                MyProceduralHitAttrs_0 candidateProceduralAttrs_0 = { int(0) };

                float _S5;

                _S5 = 0.00000000000000000000;

                MyProceduralHitAttrs_0 _S6;

                _S6 = candidateProceduralAttrs_0;

                bool _S7 = myProceduralIntersection_0(_S5, _S6);

                float tHit_1 = _S5;

                MyProceduralHitAttrs_0 candidateProceduralAttrs_1 = _S6;

                if(_S7)
                {
                    MyRayPayload_0 _S8;

                    _S8 = payload_5;

                    bool _S9 = myProceduralAnyHit_0(_S8);

                    MyRayPayload_0 _S10 = _S8;

                    if(_S9)
                    {
                        query_0.CommitProceduralPrimitiveHit(tHit_1);

                        if((bool) C_0.shouldStopAtFirstHit_0)
                        {

                            query_0.Abort();
                        }
                        else
                        {
                        }

                        committedProceduralAttrs_1 = candidateProceduralAttrs_1;
                    }
                    else
                    {
                        committedProceduralAttrs_1 = committedProceduralAttrs_0;
                    }

                    payload_6 = _S10;
                    committedProceduralAttrs_2 = committedProceduralAttrs_1;
                }
                else
                {
                    payload_6 = payload_5;
                    committedProceduralAttrs_2 = committedProceduralAttrs_0;
                }

                payload_7 = payload_6;
                committedProceduralAttrs_3 = committedProceduralAttrs_2;
                break;
            }
        case (uint) int(0):
            {
                MyRayPayload_0 _S11;
                _S11 = payload_5;

                bool _S12 = myTriangleAnyHit_0(_S11);
                MyRayPayload_0 _S13 = _S11;

                if(_S12)
                {
                    query_0.CommitNonOpaqueTriangleHit();
                    if((bool) C_0.shouldStopAtFirstHit_0)
                    {
                        query_0.Abort();
                    }
                    else
                    {
                    }
                }
                else
                {
                }

                payload_7 = _S13;
                committedProceduralAttrs_3 = committedProceduralAttrs_0;
                break;
            }
        default:
            {
                payload_7 = payload_5;
                committedProceduralAttrs_3 = committedProceduralAttrs_0;
                break;
            }
        }

        payload_5 = payload_7;
        committedProceduralAttrs_0 = committedProceduralAttrs_3;
    }

    uint _S14 = query_0.CommittedStatus();

    switch(_S14)
    {
    case (uint) int(1):
        {
            MyRayPayload_0 _S15;

            _S15 = payload_5;

            myTriangleClosestHit_0(_S15);
            break;
        }
    case (uint) int(2):
        {

            MyRayPayload_0 _S16;
            _S16 = payload_5;

            myProceduralClosestHit_0(_S16, committedProceduralAttrs_0);
            break;
        }
    case (uint) int(0):
        {
            MyRayPayload_0 _S17;

            _S17 = payload_5;

            myMiss_0(_S17);
            break;
        }
    default:
        {
            break;
        }
    }

    return;
}
