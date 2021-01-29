//TEST_IGNORE_FILE:

RaytracingAccelerationStructure gScene_0 : register(t0);

int helper_0(RayQuery<int(0) > q_0)
{
    RayDesc ray_0;
    ray_0.Origin = (vector<float,3>) int(0);
    ray_0.Direction = (vector<float,3>) int(0);
    ray_0.TMin = (float) int(0);
    ray_0.TMax = 1000.00000000000000000000;
    q_0.TraceRayInline(gScene_0, (uint) int(0), (uint) int(-1), ray_0);
    return int(1);
}

RWStructuredBuffer<int > gOutput_0 : register(u0);

[shader("compute")][numthreads(1, 1, 1)]
void computeMain(uint tid_0 : SV_DISPATCHTHREADID)
{
    RayQuery<int(0) > rayQuery_0;
    int _S1 = helper_0(rayQuery_0);

    gOutput_0[tid_0.x] = _S1;
    return;
}
