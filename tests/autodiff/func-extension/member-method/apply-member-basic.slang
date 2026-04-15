//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Test: __func_extension __apply for a member method.
// The this parameter is implicit — the apply body accesses it via `this`.

struct MyVec : IDifferentiable
{
    typedef MyVec Differential;
    float x;
    float y;

    float lengthSquared() { return x * x + y * y; }
};

struct LengthSqCtx
{
    float cx;
    float cy;
    void operator()(out MyVec dSelf, float dOut)
    {
        dSelf = MyVec(dOut * 2.0 * cx, dOut * 2.0 * cy);
    }
};

// apply for MyVec::lengthSquared — `this` is implicit
__func_extension __apply(MyVec::lengthSquared)() -> Tuple<float, LengthSqCtx>
{
    // `this` refers to the MyVec instance
    return makeTuple(this.x * this.x + this.y * this.y, LengthSqCtx(this.x, this.y));
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    let v = MyVec(3.0, 4.0);
    // lengthSquared(3,4) = 25, d/dx = 6, d/dy = 8
    var dpv = diffPair(v, MyVec(0.0, 0.0));
    bwd_diff(MyVec::lengthSquared)(dpv, 1.0);
    outputBuffer[0] = dpv.d.x;
    outputBuffer[1] = dpv.d.y;
    // CHECK: 6.0
    // CHECK: 8.0
}
