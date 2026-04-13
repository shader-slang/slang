//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Test: __func_extension fwd_diff for a static method of a differentiable type.
// Static methods have no this param regardless of whether the type is differentiable.

struct DiffVec : IDifferentiable
{
    typedef DiffVec Differential;
    float x;
    float y;

    static float dot(DiffVec a, DiffVec b)
    {
        return a.x * b.x + a.y * b.y;
    }
};

__func_extension fwd_diff(DiffVec::dot)(DifferentialPair<DiffVec> a, DifferentialPair<DiffVec> b) -> DifferentialPair<float>
{
    float p = DiffVec::dot(a.p, b.p);
    // d/d(a,b) of a.x*b.x + a.y*b.y
    float d = a.d.x * b.p.x + a.p.x * b.d.x + a.d.y * b.p.y + a.p.y * b.d.y;
    return diffPair(p, d);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    DiffVec a = { 1.0, 2.0 };
    DiffVec b = { 3.0, 4.0 };
    DiffVec da = { 1.0, 0.0 };
    DiffVec db = { 0.0, 1.0 };
    var dpa = diffPair(a, da);
    var dpb = diffPair(b, db);
    DifferentialPair<float> result = fwd_diff(DiffVec::dot)(dpa, dpb);
    // dot(a,b) = 3 + 8 = 11
    // d = da.x*b.x + a.x*db.x + da.y*b.y + a.y*db.y
    //   = 1*3 + 1*0 + 0*4 + 2*1 = 5
    outputBuffer[0] = result.p;
    outputBuffer[1] = result.d;
    // CHECK: 11.0
    // CHECK: 5.0
}
