//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Test: __func_extension bwd_diff for a static method of a differentiable type.

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

__func_extension bwd_diff(DiffVec::dot)(inout DifferentialPair<DiffVec> a, inout DifferentialPair<DiffVec> b, float dOut) -> void
{
    // d(dot)/d(a) = b, d(dot)/d(b) = a
    a = diffPair(a.p, DiffVec(dOut * b.p.x, dOut * b.p.y));
    b = diffPair(b.p, DiffVec(dOut * a.p.x, dOut * a.p.y));
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    var dpa = diffPair(DiffVec(1.0, 2.0), DiffVec(0.0, 0.0));
    var dpb = diffPair(DiffVec(3.0, 4.0), DiffVec(0.0, 0.0));
    bwd_diff(DiffVec::dot)(dpa, dpb, 1.0);
    // d(dot)/da = b = (3, 4)
    outputBuffer[0] = dpa.d.x;
    outputBuffer[1] = dpa.d.y;
    // d(dot)/db = a = (1, 2)
    outputBuffer[2] = dpb.d.x;
    outputBuffer[3] = dpb.d.y;
    // CHECK: 3.0
    // CHECK: 4.0
    // CHECK: 1.0
    // CHECK: 2.0
}
