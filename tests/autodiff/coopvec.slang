//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -render-feature cooperative-vector -output-using-type -g0

// CHECK: type: float
// CHECK-NEXT: 3.000000
// CHECK-NEXT: 5.000000
// CHECK-NEXT: 7.000000
// CHECK-NEXT: 9.000000
// CHECK-NEXT: 4.000000
// CHECK-NEXT: 60.000000
// CHECK-NEXT: 800.000000
// CHECK-NEXT: 10000.000000
// CHECK-NEXT: -2.000000
// CHECK-NEXT: -40.000000
// CHECK-NEXT: -600.000000
// CHECK-NEXT: -8000.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 4.000000
// CHECK-NEXT: 6.000000
// CHECK-NEXT: 8.000000
// CHECK-NEXT: -2.000000
// CHECK-NEXT: -3.000000
// CHECK-NEXT: -4.000000
// CHECK-NEXT: -5.000000
// CHECK-NEXT: -1.000000
// CHECK-NEXT: -10.000000
// CHECK-NEXT: -100.000000
// CHECK-NEXT: -1000.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 30.000000
// CHECK-NEXT: 400.000000
// CHECK-NEXT: 5000.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 5.135842
// CHECK-NEXT: 10.27168
// CHECK-NEXT: 15.4075
// CHECK-NEXT: 20.5433

typealias FloatCoopVec4 = CoopVec<float, 4>;
typealias DPFloatCoopVec4 = DifferentialPair<FloatCoopVec4>;

// Exercises native CoopVec arithmetic operators through the differentiable arithmetic
// interface and verifies reverse-mode gradients for both operands.
[BackwardDifferentiable]
FloatCoopVec4 testCompoundArithmetic(FloatCoopVec4 x, FloatCoopVec4 y)
{
    let sum = x + y;
    let diff = x - y;
    return sum * diff;
}

// Covers common unary math functions that now have CoopVec derivatives.
[BackwardDifferentiable]
FloatCoopVec4 testCommonMath(FloatCoopVec4 x)
{
    return exp(x) + sin(x) + cos(x);
}

// Covers quotient-rule and unary-negation derivatives for native CoopVec arithmetic.
[BackwardDifferentiable]
FloatCoopVec4 testDivisionNegation(FloatCoopVec4 x, FloatCoopVec4 y)
{
    return -(x / y);
}

// Keeps additional element-wise math derivatives covered without relying on metadata-heavy
// CoopVec operations such as matrix products.
[BackwardDifferentiable]
FloatCoopVec4 testAdditionalMath(FloatCoopVec4 x)
{
    let one = FloatCoopVec4(1.0);
    return exp2(x) + tanh(x) + log(one + x) + log2(one + x) + atan(x);
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    {
        // Compound arithmetic checks primal output and both reverse gradients.
        let x = FloatCoopVec4(2.0, 3.0, 4.0, 5.0);
        let y = FloatCoopVec4(1.0, 2.0, 3.0, 4.0);
        let dOut = FloatCoopVec4(1.0, 10.0, 100.0, 1000.0);

        let primal = testCompoundArithmetic(x, y);
        var dx = DPFloatCoopVec4(x, FloatCoopVec4(0.0, 0.0, 0.0, 0.0));
        var dy = DPFloatCoopVec4(y, FloatCoopVec4(0.0, 0.0, 0.0, 0.0));
        bwd_diff(testCompoundArithmetic)(dx, dy, dOut);

        primal.store(outputBuffer, 0);
        dx.d.store(outputBuffer, 16);
        dy.d.store(outputBuffer, 32);
    }

    {
        // At x = 0, exp(x) + sin(x) + cos(x) has a simple derivative of 2.
        let x = FloatCoopVec4(0.0, 0.0, 0.0, 0.0);
        let dOut = FloatCoopVec4(1.0, 2.0, 3.0, 4.0);

        let primal = testCommonMath(x);
        var dx = DPFloatCoopVec4(x, FloatCoopVec4(0.0, 0.0, 0.0, 0.0));
        bwd_diff(testCommonMath)(dx, dOut);

        primal.store(outputBuffer, 48);
        dx.d.store(outputBuffer, 64);
    }

    {
        // y = 1 keeps the expected division/negation gradients easy to inspect.
        let x = FloatCoopVec4(2.0, 3.0, 4.0, 5.0);
        let y = FloatCoopVec4(1.0, 1.0, 1.0, 1.0);
        let dOut = FloatCoopVec4(1.0, 10.0, 100.0, 1000.0);

        let primal = testDivisionNegation(x, y);
        var dx = DPFloatCoopVec4(x, FloatCoopVec4(0.0, 0.0, 0.0, 0.0));
        var dy = DPFloatCoopVec4(y, FloatCoopVec4(0.0, 0.0, 0.0, 0.0));
        bwd_diff(testDivisionNegation)(dx, dy, dOut);

        primal.store(outputBuffer, 80);
        dx.d.store(outputBuffer, 96);
        dy.d.store(outputBuffer, 112);
    }

    {
        // x = 0 exercises exp2/tanh/log/log2/atan derivatives at stable input values.
        let x = FloatCoopVec4(0.0, 0.0, 0.0, 0.0);
        let dOut = FloatCoopVec4(1.0, 2.0, 3.0, 4.0);

        let primal = testAdditionalMath(x);
        var dx = DPFloatCoopVec4(x, FloatCoopVec4(0.0, 0.0, 0.0, 0.0));
        bwd_diff(testAdditionalMath)(dx, dOut);

        primal.store(outputBuffer, 128);
        dx.d.store(outputBuffer, 144);
    }
}
