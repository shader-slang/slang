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

struct S
{
    row_major float4x4 a[2];
}

//TEST_INPUT:ubuffer(data=[0], stride=128):name sb
StructuredBuffer<S> sb;

[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = unconstrained(sb[0].a);
    output[1] = differentiable(sb[0].a);
    output[2] = sized(sb[0].a);
}

// CHECK: type: float
// CHECK-NEXT: 1.0
// CHECK-NEXT: 2.0
// CHECK-NEXT: 1.0
