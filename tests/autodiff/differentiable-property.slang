//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cuda

// A differentiability attribute on a property accessor must not crash semantic
// checking: declaring and reading such a getter compiles and runs. (#12210)

struct Square : IDifferentiable
{
    float v;

    property squared : float
    {
        [Differentiable]
        get { return v * v; }
    }
}

//TEST_INPUT:set output = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    Square s = {3.0};
    // CHECK: 9.0
    output[0] = s.squared;
}
