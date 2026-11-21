// Converting a `double` to `T` inside a differentiable function that is generic over
// `__BuiltinFloatingPointType` propagates the derivative of the `double` operand, both in the
// `float` and in the `double` specialization (shader-slang/slang#14062).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -output-using-type

//TEST_INPUT:ubuffer(data=[0], count=7, stride=8):out,name=outputBuffer
RWStructuredBuffer<uint64_t> outputBuffer;

// d/dx (T(x) * T(x) + T(0.1l)) = 2 * x; the `double` literal is a constant with no derivative.
[Differentiable]
T squarePlusTenth<T : __BuiltinFloatingPointType>(double x)
{
    T t = T(x);
    return t * t + T(0.1l);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // The `float` results are 9.1f (0x4111999A) and 6.0f (0x40C00000). The backward derivative
    // goes to the `double` parameter `x`, so it is 6.0 as a `double` (0x4018000000000000).
    let fwdFloat = fwd_diff(squarePlusTenth<float>)(diffPair(3.0l, 1.0l));
    // CHECK: 1091672474
    outputBuffer[0] = bit_cast<uint32_t>(fwdFloat.p);
    // CHECK-NEXT: 1086324736
    outputBuffer[1] = bit_cast<uint32_t>(fwdFloat.d);

    var bwdFloat = diffPair(3.0l);
    bwd_diff(squarePlusTenth<float>)(bwdFloat, 1.0f);
    // CHECK-NEXT: 4618441417868443648
    outputBuffer[2] = bit_cast<uint64_t>(bwdFloat.d);

    // The `double` results are 9.0 + 0.1 (not 9.0 + double(0.1f)) and 6.0.
    let fwdDouble = fwd_diff(squarePlusTenth<double>)(diffPair(3.0l, 1.0l));
    // CHECK-NEXT: 4621312462630892339
    outputBuffer[3] = bit_cast<uint64_t>(fwdDouble.p);
    // CHECK-NEXT: 4618441417868443648
    outputBuffer[4] = bit_cast<uint64_t>(fwdDouble.d);

    var bwdDouble = diffPair(3.0l);
    bwd_diff(squarePlusTenth<double>)(bwdDouble, 1.0l);
    // CHECK-NEXT: 4618441417868443648
    outputBuffer[5] = bit_cast<uint64_t>(bwdDouble.d);

    // 0.1 is not exact in `float`, so this derivative tells the paths apart: 2 * 0.1 as a `double`,
    // where rounding `T(x)` through `float` gives 4596373779801702400.
    let fwdTenth = fwd_diff(squarePlusTenth<double>)(diffPair(0.1l, 1.0l));
    // CHECK-NEXT: 4596373779694328218
    outputBuffer[6] = bit_cast<uint64_t>(fwdTenth.d);
}
