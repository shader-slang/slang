// Verify base dot-product dispatch and differentiable scalar/vector dot products at runtime.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -Xslang -experimental-feature

// CHECK: type: float
// CHECK-NEXT: 11.000000
// CHECK-NEXT: 32.000000
// CHECK-NEXT: 6.000000
// CHECK-NEXT: 29.000000
// CHECK-NEXT: 3.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 32.000000
// CHECK-NEXT: 6.000000
// CHECK-NEXT: 4.000000
// CHECK-NEXT: 2.000000

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=results

import slang.numerics.differentiable;

struct Pair : IDotProduct
{
    float first;
    float second;

    typealias Scalar = float;
    typealias Mask = bool;

    static This fromScalar(Scalar value)
    {
        return {value, value};
    }

    Scalar dotProductWith(This other)
    {
        return first * other.first + second * other.second;
    }
}

[Differentiable]
T.Scalar differentiableDot<T : IDifferentiableDotProduct>(T left, T right)
{
    return dot(left, right);
}

[Differentiable]
float scalarDot(float left, float right)
{
    return differentiableDot(left, right);
}

[Differentiable]
float vectorDot(float3 left, float3 right)
{
    return differentiableDot(left, right);
}

RWStructuredBuffer<float> results;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    results[0] = dot(Pair(1.0, 2.0), Pair(3.0, 4.0));
    results[1] = float(dot(int3(1, 2, 3), int3(4, 5, 6)));

    let scalarForward = fwd_diff(scalarDot)(diffPair(2.0, 5.0), diffPair(3.0, 7.0));
    let vectorForward = fwd_diff(vectorDot)(
        diffPair(float3(1.0, 2.0, 3.0), float3(1.0, 0.0, 0.0)),
        diffPair(float3(4.0, 5.0, 6.0), float3(0.0, 1.0, 0.0)));

    var scalarLeft = diffPair(2.0, 0.0);
    var scalarRight = diffPair(3.0, 0.0);
    bwd_diff(scalarDot)(scalarLeft, scalarRight, 1.0);

    var vectorLeft = diffPair(float3(1.0, 2.0, 3.0), float3(0.0));
    var vectorRight = diffPair(float3(4.0, 5.0, 6.0), float3(0.0));
    bwd_diff(vectorDot)(vectorLeft, vectorRight, 1.0);

    results[2] = scalarForward.p;
    results[3] = scalarForward.d;
    results[4] = scalarLeft.d;
    results[5] = scalarRight.d;
    results[6] = vectorForward.p;
    results[7] = vectorForward.d;
    results[8] = vectorLeft.d.x;
    results[9] = vectorRight.d.y;
}
