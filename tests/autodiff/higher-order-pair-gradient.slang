//TEST:SIMPLE(filecheck=CUDA): -target cuda -validate-ir
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=NUMERIC): -cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=NUMERIC): -cuda -compute -shaderobj -output-using-type

// CUDA: __global__ void computeMain(

// Runtime inputs are the coefficient, scale, x, and the reverse-mode seed.
//TEST_INPUT:ubuffer(data=[3.0 2.0 4.0 0.5 -2.0 0.5 -3.0 -1.0 0.0 -2.0 5.0 1.0 2.0 3.0 0.0 2.0], stride=4):name=testInputs
StructuredBuffer<float> testInputs;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct Coefficients : IDifferentiable
{
    float value;
}

[Differentiable]
float apply(Coefficients coefficients, float x)
{
    return coefficients.value * x;
}

struct Parameters : IDifferentiable
{
    Coefficients coefficients;
    float scale;
    // Exercise different primal and differential struct types.
    uint tag;
}

[Differentiable]
float evaluate(Parameters parameters, float x)
{
    // Keeping this call exercises reverse-gradient storage whose pair type is
    // expressed as a deferred Differential associated-type lookup during AD.
    return apply(parameters.coefficients, x) * parameters.scale * x;
}

[Differentiable]
float directionalDerivative(Parameters parameters, float x)
{
    return fwd_diff(evaluate)(diffPair(parameters), diffPair(x, 1.f)).d;
}

// For coefficient c and scale s, evaluate=c*s*x*x and its x derivative=2*c*s*x.
// A reverse seed q produces dc=2*s*x*q, ds=2*c*x*q, and dx=2*c*s*q.
// Check the forward value, all three gradients, and preserved c/s/x primals.

// NUMERIC: {{^}}48.000000{{$}}
// NUMERIC-NEXT: {{^}}8.000000{{$}}
// NUMERIC-NEXT: {{^}}12.000000{{$}}
// NUMERIC-NEXT: {{^}}6.000000{{$}}
// NUMERIC-NEXT: {{^}}3.000000{{$}}
// NUMERIC-NEXT: {{^}}2.000000{{$}}
// NUMERIC-NEXT: {{^}}4.000000{{$}}
// NUMERIC-NEXT: {{^}}6.000000{{$}}
// NUMERIC-NEXT: {{^}}3.000000{{$}}
// NUMERIC-NEXT: {{^}}-12.000000{{$}}
// NUMERIC-NEXT: {{^}}2.000000{{$}}
// NUMERIC-NEXT: {{^}}-2.000000{{$}}
// NUMERIC-NEXT: {{^}}0.500000{{$}}
// NUMERIC-NEXT: {{^}}-3.000000{{$}}
// NUMERIC-NEXT: {{^-?0.000000$}}
// NUMERIC-NEXT: {{^}}-20.000000{{$}}
// NUMERIC-NEXT: {{^-?0.000000$}}
// NUMERIC-NEXT: {{^-?0.000000$}}
// NUMERIC-NEXT: {{^-?0.000000$}}
// NUMERIC-NEXT: {{^}}-2.000000{{$}}
// NUMERIC-NEXT: {{^}}5.000000{{$}}
// NUMERIC-NEXT: {{^-?0.000000$}}
// NUMERIC-NEXT: {{^-?0.000000$}}
// NUMERIC-NEXT: {{^-?0.000000$}}
// NUMERIC-NEXT: {{^}}24.000000{{$}}
// NUMERIC-NEXT: {{^}}2.000000{{$}}
// NUMERIC-NEXT: {{^}}3.000000{{$}}
// NUMERIC-NEXT: {{^-?0.000000$}}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    for (uint i = 0; i < 4; ++i)
    {
        Parameters parameters = {{testInputs[4 * i]}, testInputs[4 * i + 1], 7u};
        float x = testInputs[4 * i + 2];
        float seed = testInputs[4 * i + 3];
        var dp = diffPair(parameters);
        var dx = diffPair(x);
        bwd_diff(directionalDerivative)(dp, dx, seed);
        outputBuffer[7 * i] = directionalDerivative(parameters, x);
        outputBuffer[7 * i + 1] = dp.d.coefficients.value;
        outputBuffer[7 * i + 2] = dp.d.scale;
        outputBuffer[7 * i + 3] = dx.d;
        outputBuffer[7 * i + 4] = dp.p.coefficients.value;
        outputBuffer[7 * i + 5] = dp.p.scale;
        outputBuffer[7 * i + 6] = dx.p;
    }
}
