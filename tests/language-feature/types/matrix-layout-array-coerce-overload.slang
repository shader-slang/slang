// Regression for shader-slang/slang#13376: overload resolution when an argument is an array of
// matrices whose layout differs from the parameter's. The conversion costs the same as the one for
// a single matrix, so a sized parameter is preferred, and an unconstrained generic loses. A generic
// constrained to `IDifferentiable` takes the argument with no conversion, so it wins over a
// conversion.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj -xslang -matrix-layout-row-major
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -output-using-type -shaderobj -xslang -matrix-layout-column-major

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name output
RWStructuredBuffer<float> output;

float unconstrained(float4x4 x[2]) { return 1; }
float unconstrained<T>(T x) { return 2; }

float differentiable(float4x4 x[2]) { return 1; }
float differentiable<T : IDifferentiable>(T x) { return 2; }

float sized(float4x4 x[2]) { return 1; }
float sized(float4x4 x[]) { return 2; }

[numthreads(1, 1, 1)]
void computeMain()
{
    row_major float4x4 a[2] = { float4x4(1), float4x4(2) };
    output[0] = unconstrained(a);
    output[1] = differentiable(a);
    output[2] = sized(a);
}

// CHECK: type: float
// CHECK-NEXT: 1.0
// CHECK-NEXT: 2.0
// CHECK-NEXT: 1.0
