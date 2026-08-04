//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -render-feature cooperative-vector -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -render-feature cooperative-vector -dx12-experimental -output-using-type -profile cs_6_9 -Xdxc -Vd
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Verify automatic differentiation through the CoopVec subscript getter and setter for each
// floating-point element type used by cooperative-vector training code.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=testResult
RWStructuredBuffer<uint> testResult;

static const int VectorSize = 3;

[Differentiable]
T readElement<T : __BuiltinFloatingPointType>(CoopVec<T, VectorSize> value)
{
    return value[1];
}

[Differentiable]
CoopVec<T, VectorSize> writeElement<T : __BuiltinFloatingPointType>(
    CoopVec<T, VectorSize> value,
    T newValue)
{
    value[1] = newValue;
    return value;
}

[Differentiable]
float sumRepeatedElements(CoopVec<float, VectorSize> value)
{
    return value[0] + value[1] + value[1];
}

bool[4] testFloatSubscript()
{
    let value = CoopVec<float, VectorSize>(1.0, 2.0, 3.0);
    let valueDiff = CoopVec<float, VectorSize>(2.0, 3.0, 5.0);
    let outputDiff = CoopVec<float, VectorSize>(2.0, 3.0, 5.0);

    bool results[4];

    let getterForwardResult = fwd_diff(readElement<float>)(diffPair(value, valueDiff));
    results[0] = getterForwardResult.p == 2.0 && getterForwardResult.d == 3.0;

    var getterBackwardValue = diffPair(value);
    bwd_diff(readElement<float>)(getterBackwardValue, 1.0);
    results[1] = getterBackwardValue.d[0] == 0.0 &&
                 getterBackwardValue.d[1] == 1.0 &&
                 getterBackwardValue.d[2] == 0.0;

    let setterForwardResult =
        fwd_diff(writeElement<float>)(diffPair(value, valueDiff), diffPair(4.0, 7.0));
    results[2] = setterForwardResult.p[0] == 1.0 &&
                 setterForwardResult.p[1] == 4.0 &&
                 setterForwardResult.p[2] == 3.0 &&
                 setterForwardResult.d[0] == 2.0 &&
                 setterForwardResult.d[1] == 7.0 &&
                 setterForwardResult.d[2] == 5.0;

    var setterBackwardValue = diffPair(value);
    var newValue = diffPair(4.0);
    bwd_diff(writeElement<float>)(setterBackwardValue, newValue, outputDiff);
    results[3] = setterBackwardValue.d[0] == 2.0 &&
                 setterBackwardValue.d[1] == 0.0 &&
                 setterBackwardValue.d[2] == 5.0 &&
                 newValue.d == 3.0;

    return results;
}

bool[4] testHalfSubscript()
{
    let value = CoopVec<half, VectorSize>(1.0h, 2.0h, 3.0h);
    let valueDiff = CoopVec<half, VectorSize>(2.0h, 3.0h, 5.0h);
    let outputDiff = CoopVec<half, VectorSize>(2.0h, 3.0h, 5.0h);

    bool results[4];

    let getterForwardResult = fwd_diff(readElement<half>)(diffPair(value, valueDiff));
    results[0] = getterForwardResult.p == 2.0h && getterForwardResult.d == 3.0h;

    var getterBackwardValue = diffPair(value);
    bwd_diff(readElement<half>)(getterBackwardValue, 1.0h);
    results[1] = getterBackwardValue.d[0] == 0.0h &&
                 getterBackwardValue.d[1] == 1.0h &&
                 getterBackwardValue.d[2] == 0.0h;

    let setterForwardResult =
        fwd_diff(writeElement<half>)(diffPair(value, valueDiff), diffPair(4.0h, 7.0h));
    results[2] = setterForwardResult.p[0] == 1.0h &&
                 setterForwardResult.p[1] == 4.0h &&
                 setterForwardResult.p[2] == 3.0h &&
                 setterForwardResult.d[0] == 2.0h &&
                 setterForwardResult.d[1] == 7.0h &&
                 setterForwardResult.d[2] == 5.0h;

    var setterBackwardValue = diffPair(value);
    var newValue = diffPair(4.0h);
    bwd_diff(writeElement<half>)(setterBackwardValue, newValue, outputDiff);
    results[3] = setterBackwardValue.d[0] == 2.0h &&
                 setterBackwardValue.d[1] == 0.0h &&
                 setterBackwardValue.d[2] == 5.0h &&
                 newValue.d == 3.0h;

    return results;
}

[Differentiable]
float[VectorSize] roundTripHalfCoopVec(float[VectorSize] input)
{
    CoopVec<half, VectorSize> value;
    [ForceUnroll]
    for (int i = 0; i < VectorSize; ++i)
        value[i] = half(input[i]);

    float[VectorSize] result;
    [ForceUnroll]
    for (int i = 0; i < VectorSize; ++i)
        result[i] = float(value[i]);
    return result;
}

bool testArrayConversion()
{
    float[VectorSize] input = {1.0, 2.0, 3.0};
    float[VectorSize] outputDiff = {2.0, 3.0, 5.0};
    var inputPair = diffPair(input);
    bwd_diff(roundTripHalfCoopVec)(inputPair, outputDiff);
    return inputPair.d[0] == 2.0 && inputPair.d[1] == 3.0 && inputPair.d[2] == 5.0;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    let floatResults = testFloatSubscript();
    let halfResults = testHalfSubscript();

    for (int i = 0; i < 4; ++i)
    {
        testResult[i] = floatResults[i];
        testResult[i + 4] = halfResults[i];
    }
    testResult[8] = testArrayConversion();

    var repeatedValue = diffPair(CoopVec<float, VectorSize>(1.0, 2.0, 3.0));
    bwd_diff(sumRepeatedElements)(repeatedValue, 1.0);
    testResult[9] = repeatedValue.d[0] == 1.0 &&
                    repeatedValue.d[1] == 2.0 &&
                    repeatedValue.d[2] == 0.0;

    // CHECK: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
}
