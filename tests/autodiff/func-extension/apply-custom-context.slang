//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Test: verify that apply's custom context is ACTUALLY used, not auto-synthesis.
// The context stores a deliberately wrong "cached" value (999.0) that the
// backward pass uses. If auto-synthesis kicked in instead, it would compute
// the correct mathematical derivative (2*x = 6.0), not our intentionally
// wrong value.

float sqr(float x) { return x * x; }

struct SqrContext
{
    float magic; // deliberately NOT the correct cached intermediate

    void operator()(out float dx, float dOut)
    {
        // Use the magic value instead of the mathematically correct derivative.
        // Correct derivative of x^2 at x=3 would be 2*3=6, but we return
        // dOut * magic = 1.0 * 999.0 = 999.0
        dx = dOut * magic;
    }
};

__func_extension apply(sqr)(float x) -> Tuple<float, SqrContext>
{
    // Return the correct primal value, but store a bogus context.
    return makeTuple(x * x, SqrContext(999.0));
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    var dpx = diffPair(3.0, 0.0);
    bwd_diff(sqr)(dpx, 1.0);

    // If our custom apply is used: dpx.d = 999.0 (from the magic context)
    // If auto-synthesis were used: dpx.d = 6.0 (correct derivative 2*x)
    outputBuffer[0] = dpx.d;
    // CHECK: 999.0
}
