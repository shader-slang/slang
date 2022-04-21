//DISABLE_TEST:SIMPLE: -target hlsl -entry computeMain -stage compute

// This test crashes when enabled, with a stack overrun. The crash happens when lowering into IR.
// NOTE! That the code has a bug in that the mul function calls itself - so would exhaust the stack *when run*.
// This exhausts the stack when compiling which shouldn't happen.

RWStructuredBuffer<float> outputBuffer;

struct SomeType
{
    float2x2 v;
};

float mul(SomeType a, float2 v) { return mul(a, v).x; }

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;

    float inVal = float(tid);

    float2 v = float2(inVal, inVal + 2);
    SomeType t = { inVal + 1, 0, 1, 0 };
    
    outputBuffer[tid] = mul(t, v).x;
} 