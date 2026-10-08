//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Use the CPU target for runtime coverage that the floating-point derivative extensions also
// specialize correctly for double.

[Differentiable]
double readElement(CoopVec<double, 3> value)
{
    return value[1];
}

[Differentiable]
CoopVec<double, 3> writeElement(CoopVec<double, 3> value, double newValue)
{
    value[1] = newValue;
    return value;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    let value = CoopVec<double, 3>(1.0, 2.0, 3.0);
    let valueDiff = CoopVec<double, 3>(2.0, 3.0, 5.0);
    let outputDiff = CoopVec<double, 3>(2.0, 3.0, 5.0);

    let getterForward = fwd_diff(readElement)(diffPair(value, valueDiff));
    outputBuffer[0] = getterForward.p == 2.0 && getterForward.d == 3.0;

    var getterBackwardValue = diffPair(value);
    bwd_diff(readElement)(getterBackwardValue, 1.0);
    outputBuffer[1] = getterBackwardValue.d[0] == 0.0 &&
                      getterBackwardValue.d[1] == 1.0 &&
                      getterBackwardValue.d[2] == 0.0;

    let setterForward =
        fwd_diff(writeElement)(diffPair(value, valueDiff), diffPair(double(4), double(7)));
    outputBuffer[2] = setterForward.p[1] == 4.0 && setterForward.d[1] == 7.0;

    var setterBackwardValue = diffPair(value);
    var newValue = diffPair(double(4));
    bwd_diff(writeElement)(setterBackwardValue, newValue, outputDiff);
    outputBuffer[3] = setterBackwardValue.d[0] == 2.0 &&
                      setterBackwardValue.d[1] == 0.0 &&
                      setterBackwardValue.d[2] == 5.0 && newValue.d == 3.0;

    // CHECK: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
    // CHECK-NEXT: 1
}
