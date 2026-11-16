//TEST:SIMPLE(filecheck=CUDA): -target cuda -validate-ir
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=NUMERIC): -cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=NUMERIC): -cuda -compute -shaderobj -output-using-type

// CUDA: __global__ void computeMain(

// Runtime inputs are the coefficient, x, and the reverse-mode seed.
//TEST_INPUT:ubuffer(data=[3.0 2.0 0.5 -2.0 -1.0 -3.0 0.0 5.0 1.0 2.0 3.0 0.0], stride=4):name=testInputs
StructuredBuffer<float> testInputs;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct Polynomial : IDifferentiable
{
    float coefficient;
    // Ensure Polynomial.Differential is distinct from Polynomial.
    uint tag;

    [Differentiable]
    float evaluate(float x)
    {
        return coefficient * x * x;
    }
}

[Differentiable]
float evaluate(no_diff Polynomial polynomial, float x)
{
    // Forward AD constructs a pair for the method receiver even though the
    // wrapper parameter itself has no differential.
    return polynomial.evaluate(x);
}

[Differentiable]
float directionalDerivative(Polynomial polynomial, float x)
{
    return fwd_diff(evaluate)(polynomial, diffPair(x, 1.f)).d;
}

// The forward derivative is 2*a*x. Reverse differentiation with seed q gives
// dx=2*a*q, while the no_diff parameter keeps da=0 in the outer derivative too.
// Check the forward value, both gradients, and preserved coefficient/x primals.

// NUMERIC: {{^}}12.000000{{$}}
// NUMERIC-NEXT: {{^}}3.000000{{$}}
// NUMERIC-NEXT: {{^}}0.000000{{$}}
// NUMERIC-NEXT: {{^}}3.000000{{$}}
// NUMERIC-NEXT: {{^}}2.000000{{$}}
// NUMERIC-NEXT: {{^}}4.000000{{$}}
// NUMERIC-NEXT: {{^}}12.000000{{$}}
// NUMERIC-NEXT: {{^}}0.000000{{$}}
// NUMERIC-NEXT: {{^}}-2.000000{{$}}
// NUMERIC-NEXT: {{^}}-1.000000{{$}}
// NUMERIC-NEXT: {{^}}0.000000{{$}}
// NUMERIC-NEXT: {{^}}0.000000{{$}}
// NUMERIC-NEXT: {{^}}0.000000{{$}}
// NUMERIC-NEXT: {{^}}0.000000{{$}}
// NUMERIC-NEXT: {{^}}5.000000{{$}}
// NUMERIC-NEXT: {{^}}12.000000{{$}}
// NUMERIC-NEXT: {{^}}0.000000{{$}}
// NUMERIC-NEXT: {{^}}0.000000{{$}}
// NUMERIC-NEXT: {{^}}2.000000{{$}}
// NUMERIC-NEXT: {{^}}3.000000{{$}}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    for (uint i = 0; i < 4; ++i)
    {
        Polynomial polynomial = {testInputs[3 * i], 7u};
        float x = testInputs[3 * i + 1];
        float seed = testInputs[3 * i + 2];
        var dp = diffPair(polynomial);
        var dx = diffPair(x);
        bwd_diff(directionalDerivative)(dp, dx, seed);
        outputBuffer[5 * i] = directionalDerivative(polynomial, x);
        outputBuffer[5 * i + 1] = dx.d;
        outputBuffer[5 * i + 2] = dp.d.coefficient;
        outputBuffer[5 * i + 3] = dp.p.coefficient;
        outputBuffer[5 * i + 4] = dx.p;
    }
}
