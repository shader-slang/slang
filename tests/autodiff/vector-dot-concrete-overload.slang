// Verify that a concrete floating-point vector dot call still binds to the core overload carrying
// the custom forward and backward derivative registrations.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// CHECK: type: int32_t
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=results

bool approximatelyEqual(float actual, float expected)
{
    return abs(actual - expected) < 0.001;
}

[Differentiable]
float concreteVectorDot(float3 left, float3 right)
{
    return dot(left, right);
}

RWStructuredBuffer<int> results;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    let forward = fwd_diff(concreteVectorDot)(
        diffPair(float3(1.0, 2.0, 3.0), float3(1.0, 0.0, 0.0)),
        diffPair(float3(4.0, 5.0, 6.0), float3(0.0, 1.0, 0.0)));

    var reverseLeft = diffPair(float3(1.0, 2.0, 3.0), float3(0.0));
    var reverseRight = diffPair(float3(4.0, 5.0, 6.0), float3(0.0));
    bwd_diff(concreteVectorDot)(reverseLeft, reverseRight, 1.0);

    results[0] = int(approximatelyEqual(forward.p, 32.0));
    results[1] = int(approximatelyEqual(forward.d, 6.0));
    results[2] = int(approximatelyEqual(reverseLeft.d.x, 4.0));
    results[3] = int(approximatelyEqual(reverseLeft.d.z, 6.0));
    results[4] = int(approximatelyEqual(reverseRight.d.y, 2.0));
}
