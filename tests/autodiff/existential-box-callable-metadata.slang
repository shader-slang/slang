//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -xslang -std -xslang legacy -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -xslang -std -xslang 2025 -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -xslang -std -xslang 2026 -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -xslang -std -xslang 202c -output-using-type

interface IValue : IDifferentiable
{
    [Differentiable]
    float get();
}

struct Value : IValue
{
    float value;

    [Differentiable]
    __init(float value)
    {
        this.value = value;
    }

    [Differentiable]
    float get()
    {
        return value;
    }
}

[Differentiable]
dyn IValue makeBox(float value)
{
    return Value(value);
}

[Differentiable]
float readBox(dyn IValue value)
{
    return value.get();
}

[Differentiable]
float evaluate(float value)
{
    dyn IValue boxed = makeBox(value);
    return readBox(boxed);
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Direct derivative members use callable metadata for the boxed result and parameter.
    outputBuffer[0] = makeBox.fwd_diff(diffPair(3.0, 1.0)).p.get();
    outputBuffer[1] = readBox.fwd_diff(makeBox.fwd_diff(diffPair(3.0, 1.0))).d;
    var pair = diffPair(3.0, 0.0);
    bwd_diff(evaluate)(pair, 1.0);
    outputBuffer[2] = pair.d;
}

// CHECK: 3.0
// CHECK: 1.0
// CHECK: 1.0
