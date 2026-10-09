// A user `diffPair` does not replace the one that pairs a differentiable receiver.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)

struct Scaled : IDifferentiable
{
    float k;

    [Differentiable]
    float eval(float t) { return k * t * t; }
}

int diffPair(Scaled s) { return 0; }

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Scaled s;
    s.k = 5.0;
    // CHECK: 20.0
    outputBuffer[0] = __fwd_diff(s.eval)(DifferentialPair<float>(2.0, 1.0)).d;
    var p = DifferentialPair<float>(2.0, 0.0);
    __bwd_diff(s.eval)(p, 1.0);
    // CHECK: 20.0
    outputBuffer[1] = p.d;
}
