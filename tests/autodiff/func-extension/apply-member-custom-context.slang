//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Test: verify that apply's custom context is ACTUALLY used for a member method.
// The context stores a deliberately wrong value — if auto-synthesis kicked in,
// we'd get the correct derivative, not our intentional magic number.

struct MyVec : IDifferentiable
{
    typedef MyVec Differential;
    float x;
    float y;

    float lengthSquared() { return x * x + y * y; }
};

struct MagicCtx
{
    void operator()(out MyVec dSelf, float dOut)
    {
        // Return a deliberately wrong derivative to prove the custom context is used.
        dSelf = MyVec(777.0, 888.0);
    }
};

__func_extension apply(MyVec::lengthSquared)() -> Tuple<float, MagicCtx>
{
    return makeTuple(this.x * this.x + this.y * this.y, MagicCtx());
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    let v = MyVec(3.0, 4.0);
    var dpv = diffPair(v, MyVec(0.0, 0.0));
    bwd_diff(MyVec::lengthSquared)(dpv, 1.0);

    // If custom apply is used: dpv.d = (777, 888)
    // If auto-synthesis were used: dpv.d = (6, 8)
    outputBuffer[0] = dpv.d.x;
    outputBuffer[1] = dpv.d.y;
    // CHECK: 777.0
    // CHECK: 888.0
}
