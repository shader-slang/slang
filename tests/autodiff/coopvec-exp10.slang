//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -render-feature cooperative-vector -output-using-type -g0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -render-feature cooperative-vector -dx12-experimental -output-using-type -profile cs_6_9 -Xdxc -Vd

// CHECK: type: float
// CHECK-NEXT: 0.100000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 10.000000
// CHECK-NEXT: 100.000000
// CHECK-NEXT: 0.23025
// CHECK-NEXT: 4.60517
// CHECK-NEXT: 69.0775
// CHECK-NEXT: 921.034
// CHECK-NEXT: 0.23025
// CHECK-NEXT: 4.60517
// CHECK-NEXT: 69.0775
// CHECK-NEXT: 921.034

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
    let seed = FloatCoopVec4(1.0, 2.0, 3.0, 4.0);

    let primal = differentiableExp10(value);
    let forward = fwd_diff(differentiableExp10)(diffPair(value, seed));

    var reverseInput = DPFloatCoopVec4(value, FloatCoopVec4(0.0));
    bwd_diff(differentiableExp10)(reverseInput, seed);

    primal.store(outputBuffer, 0);
    forward.d.store(outputBuffer, 16);
    reverseInput.d.store(outputBuffer, 32);
}
