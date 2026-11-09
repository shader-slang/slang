//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -render-feature cooperative-vector -output-using-type -g0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -render-feature cooperative-vector -dx12-experimental -output-using-type -profile cs_6_9 -Xdxc -Vd

// CHECK: type: float
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 0.230
// CHECK-NEXT: 4.605
// CHECK-NEXT: 69.07
// CHECK-NEXT: 921.0
// CHECK-NEXT: 0.921
// CHECK-NEXT: 6.907
// CHECK-NEXT: 46.05
// CHECK-NEXT: 230.2

typealias FloatCoopVec4 = CoopVec<float, 4>;
typealias DPFloatCoopVec4 = DifferentialPair<FloatCoopVec4>;

[Differentiable]
FloatCoopVec4 differentiableExp10(FloatCoopVec4 value)
{
    return exp10(value);
}

//TEST_INPUT:ubuffer(data=[0], stride=4, count=12):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    let value = FloatCoopVec4(-1.0, 0.0, 1.0, 2.0);
    let forwardSeed = FloatCoopVec4(1.0, 2.0, 3.0, 4.0);
    let reverseSeed = FloatCoopVec4(4.0, 3.0, 2.0, 1.0);

    let primal = differentiableExp10(FloatCoopVec4(0.0));
    let forward = fwd_diff(differentiableExp10)(diffPair(value, forwardSeed));

    var reverseInput = DPFloatCoopVec4(value, FloatCoopVec4(0.0));
    bwd_diff(differentiableExp10)(reverseInput, reverseSeed);

    primal.store(outputBuffer, 0);
    forward.d.store(outputBuffer, 16);
    reverseInput.d.store(outputBuffer, 32);
}
