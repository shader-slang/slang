//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

struct Values : IDifferentiablePtrType
{
    struct Gradients : IDifferentiablePtrType
    {
        typealias Differential = Gradients;
        no_diff RWStructuredBuffer<float> buffer;
    }
    typealias Differential = Gradients;
    no_diff RWStructuredBuffer<float> buffer;

    [ForwardDerivative(loadForward)]
    [BackwardDerivative(loadBackward)]
    float load(uint index) { return buffer[index]; }

    static DifferentialPair<float> loadForward(DifferentialPtrPair<Values> self, uint index)
    {
        return diffPair(self.p.buffer[index], self.d.buffer[index]);
    }

    static void loadBackward(DifferentialPtrPair<Values> self, uint index, float gradient)
    {
        self.d.buffer[index] += gradient;
    }
}

[Differentiable]
float applyIndexed<F : IDifferentiableFuncWithDiffThis<float, uint, float>>(
    F function, uint index, float value)
{
    return function(index, value);
}

[Differentiable]
float evaluate(Values values, uint index, float value, no_diff float bias)
{
    return applyIndexed(
        (uint i, float v)~ => values.load(i) * v * v + bias,
        index,
        value);
}

[Differentiable]
float evaluateNested(Values values, uint index, float value)
{
    let inner = (uint i, float v)~ => values.load(i) * v;
    return applyIndexed((uint i, float v)~ => inner(i, v) * v, index, value);
}

[Differentiable]
float evaluateEmpty(uint index, float value)
{
    return applyIndexed((uint i, float v)~ => float(i) * v, index, value);
}

//TEST_INPUT: set parameters = ubuffer(data=[3.0 5.0], stride=4)
//TEST_INPUT: set gradients = ubuffer(data=[0.0 0.0], stride=4)
//TEST_INPUT: set output = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<float> parameters;
RWStructuredBuffer<float> gradients;
RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    let values = DifferentialPtrPair<Values>(Values(parameters), Values::Gradients(gradients));
    var value = diffPair(4.0);
    bwd_diff(evaluate)(values, 1, value, 2.0, 1.0);
    output[0] = value.d;
    output[1] = gradients[1];
    output[2] = evaluate(values.p, 1, 4.0, 2.0);
    // CHECK: 40.0
    // CHECK: 16.0
    // CHECK: 82.0

    gradients[1] = 0.0;
    value = diffPair(4.0);
    bwd_diff(evaluateNested)(values, 1, value, 1.0);
    output[3] = value.d;
    output[4] = gradients[1];
    // CHECK: 40.0
    // CHECK: 16.0

    // An empty pointer differential is valid for a capture-free differentiable lambda.
    output[5] = applyIndexed((uint i, float v)~ => float(i) * v, 2, 3.0);
    // CHECK: 6.0

    gradients[1] = 2.0;
    output[6] = fwd_diff(evaluate)(values, 1, diffPair(4.0, 1.0), 2.0).d;
    // CHECK: 72.0

    value = diffPair(3.0);
    bwd_diff(evaluateEmpty)(2, value, 1.0);
    output[7] = value.d;
    // CHECK: 2.0
}
