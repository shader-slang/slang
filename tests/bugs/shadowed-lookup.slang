//TEST:COMPARE_COMPUTE_EX: -slang -compute

RWStructuredBuffer<float> outputBuffer;

struct SomeType
{
    float2x2 v;
};

float mul(SomeType a, float2 v) { return mul(a.v, v).x; }

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;

    float inVal = float(tid);

    float2 v = float2(inVal, inVal + 2);
    SomeType t = { inVal + 1, 0, 1, 0 };
    
    outputBuffer[tid] = mul(t, v).x;
} 