// Verify base dot-product dispatch and differentiable scalar/vector dot products at runtime.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -Xslang -experimental-feature

// CHECK: type: int32_t
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=results

import slang.numerics.differentiable;

bool approximatelyEqual(float actual, float expected)
{
    return abs(actual - expected) < 0.001;
}

struct Pair : IDifferentiableDotProduct
{
    float first;
    float second;

    typealias Differential = Pair;
    typealias Scalar = float;
    typealias Mask = bool;

    [Differentiable]
    static Differential dzero()
    {
        return {0.0, 0.0};
    }

    [Differentiable]
    static Differential dadd(Differential left, Differential right)
    {
        return {left.first + right.first, left.second + right.second};
    }

    [Differentiable]
    static This fromScalar(Scalar value)
    {
        return {value, value};
    }

    Scalar dotProductWith(This other)
    {
        return first * other.first + second * other.second;
    }

    static DifferentialPair<Scalar> forwardDifferentiateDotProduct(
        DifferentialPair<This> left,
        DifferentialPair<This> right)
    {
        return diffPair(
            left.p.dotProductWith(right.p),
            left.d.dotProductWith(right.p) + left.p.dotProductWith(right.d));
    }

    static void backwardDifferentiateDotProduct(
        inout DifferentialPair<This> left,
        inout DifferentialPair<This> right,
        Scalar.Differential outputDifferential)
    {
        let leftPrimal = left.p;
        let rightPrimal = right.p;
        left = diffPair(
            leftPrimal,
            Pair(
                rightPrimal.first * outputDifferential,
                rightPrimal.second * outputDifferential));
        right = diffPair(
            rightPrimal,
            Pair(
                leftPrimal.first * outputDifferential,
                leftPrimal.second * outputDifferential));
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

[Differentiable]
float pairDot(Pair left, Pair right)
{
    return differentiableDot(left, right);
}

RWStructuredBuffer<int> results;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    results[0] = int(approximatelyEqual(dot(Pair(1.0, 2.0), Pair(3.0, 4.0)), 11.0));
    results[1] = int(dot(int3(1, 2, 3), int3(4, 5, 6)) == 32);

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

    let pairForward = fwd_diff(pairDot)(
        diffPair(Pair(1.0, 2.0), Pair(5.0, 0.0)),
        diffPair(Pair(3.0, 4.0), Pair(0.0, 7.0)));
    var pairLeft = diffPair(Pair(1.0, 2.0), Pair.dzero());
    var pairRight = diffPair(Pair(3.0, 4.0), Pair.dzero());
    bwd_diff(pairDot)(pairLeft, pairRight, 1.0);

    results[2] = int(approximatelyEqual(scalarForward.p, 6.0));
    results[3] = int(approximatelyEqual(scalarForward.d, 29.0));
    results[4] = int(approximatelyEqual(scalarLeft.d, 3.0));
    results[5] = int(approximatelyEqual(scalarRight.d, 2.0));
    results[6] = int(approximatelyEqual(vectorForward.p, 32.0));
    results[7] = int(approximatelyEqual(vectorForward.d, 6.0));
    results[8] = int(approximatelyEqual(vectorLeft.d.x, 4.0));
    results[9] = int(approximatelyEqual(vectorRight.d.y, 2.0));
    results[10] = int(approximatelyEqual(pairForward.p, 11.0));
    results[11] = int(approximatelyEqual(pairForward.d, 29.0));
    results[12] = int(approximatelyEqual(pairLeft.d.first, 3.0));
    results[13] = int(approximatelyEqual(pairRight.d.second, 2.0));
}
