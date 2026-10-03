//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -render-feature cooperative-vector -output-using-type -g0
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -render-feature cooperative-vector -dx12-experimental -output-using-type -profile cs_6_9 -Xdxc -Vd
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

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
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 5.000000
// CHECK-NEXT: 11.000000
// CHECK-NEXT: 19.000000
// CHECK-NEXT: 3.000000
// CHECK-NEXT: 11.000000
// CHECK-NEXT: 23.000000
// CHECK-NEXT: 39.000000
// CHECK-NEXT: 9.000000
// CHECK-NEXT: 16.000000
// CHECK-NEXT: 25.000000
// CHECK-NEXT: 36.000000
// CHECK-NEXT: 6.000000
// CHECK-NEXT: 80.000000
// CHECK-NEXT: 1000.000000
// CHECK-NEXT: 12000.000000
// CHECK-NEXT: 7.500000
// CHECK-NEXT: 8.000000
// CHECK-NEXT: 19.000000
// CHECK-NEXT: 34.000000
// CHECK-NEXT: 3.000000
// CHECK-NEXT: 30.000000
// CHECK-NEXT: 500.000000
// CHECK-NEXT: 7000.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 30.000000
// CHECK-NEXT: 400.000000
// CHECK-NEXT: 5000.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 4.000000
// CHECK-NEXT: 6.000000
// CHECK-NEXT: 8.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 20.000000
// CHECK-NEXT: 200.000000
// CHECK-NEXT: 2000.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 4.000000
// CHECK-NEXT: 6.000000
// CHECK-NEXT: 8.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 20.000000
// CHECK-NEXT: 200.000000
// CHECK-NEXT: 2000.000000
// CHECK-NEXT: 3.000000
// CHECK-NEXT: 5.000000
// CHECK-NEXT: 7.000000
// CHECK-NEXT: 9.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 8.000000
// CHECK-NEXT: 18.000000
// CHECK-NEXT: 32.000000
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
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 5.135842
// CHECK-NEXT: 10.27168
// CHECK-NEXT: 15.4075
// CHECK-NEXT: 20.5433
// CHECK-NEXT: 9.000000
// CHECK-NEXT: 16.000000
// CHECK-NEXT: 25.000000
// CHECK-NEXT: 36.000000
// CHECK-NEXT: 6.000000
// CHECK-NEXT: 16.000000
// CHECK-NEXT: 30.000000
// CHECK-NEXT: 48.000000
// CHECK-NEXT: 7.500000
// CHECK-NEXT: 8.000000
// CHECK-NEXT: 19.000000
// CHECK-NEXT: 34.000000
// CHECK-NEXT: 5.000000
// CHECK-NEXT: 9.000000
// CHECK-NEXT: 19.000000
// CHECK-NEXT: 33.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 4.000000
// CHECK-NEXT: 6.000000
// CHECK-NEXT: 8.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 20.000000
// CHECK-NEXT: 200.000000
// CHECK-NEXT: 2000.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 4.000000
// CHECK-NEXT: 6.000000
// CHECK-NEXT: 8.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 20.000000
// CHECK-NEXT: 200.000000
// CHECK-NEXT: 2000.000000
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 10.000000
// CHECK-NEXT: 100.000000
// CHECK-NEXT: 1000.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 4.000000
// CHECK-NEXT: 6.000000
// CHECK-NEXT: 8.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 20.000000
// CHECK-NEXT: 200.000000
// CHECK-NEXT: 2000.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 4.000000
// CHECK-NEXT: 6.000000
// CHECK-NEXT: 8.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 20.000000
// CHECK-NEXT: 200.000000
// CHECK-NEXT: 2000.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 4.000000
// CHECK-NEXT: 6.000000
// CHECK-NEXT: 8.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 20.000000
// CHECK-NEXT: 200.000000
// CHECK-NEXT: 2000.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 4.000000
// CHECK-NEXT: 6.000000
// CHECK-NEXT: 8.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 20.000000
// CHECK-NEXT: 200.000000
// CHECK-NEXT: 2000.000000

typealias FloatCoopVec4 = CoopVec<float, 4>;
typealias DPFloatCoopVec4 = DifferentialPair<FloatCoopVec4>;

// Exercises native CoopVec arithmetic operators through the differentiable arithmetic
// interface and verifies gradients for both operands in reverse and forward mode.
[Differentiable]
FloatCoopVec4 testCompoundArithmetic(FloatCoopVec4 x, FloatCoopVec4 y)
{
    let sum = x + y;
    let diff = x - y;
    return sum * diff;
}

// Covers common unary math functions that now have CoopVec derivatives.
[Differentiable]
FloatCoopVec4 testCommonMath(FloatCoopVec4 x)
{
    return exp(x) + sin(x) + cos(x);
}

// Covers quotient-rule and unary-negation derivatives for native CoopVec arithmetic.
[Differentiable]
FloatCoopVec4 testDivisionNegation(FloatCoopVec4 x, FloatCoopVec4 y)
{
    return -(x / y);
}

// Keeps additional element-wise math derivatives covered without relying on metadata-heavy
// CoopVec operations such as matrix products.
[Differentiable]
FloatCoopVec4 testAdditionalMath(FloatCoopVec4 x)
{
    let one = FloatCoopVec4(1.0);
    return exp2(x) + tanh(x) + log(one + x) + log2(one + x) + atan(x);
}

// Covers forward-mode arithmetic derivatives on native CoopVec operands.
[Differentiable]
FloatCoopVec4 testForwardArithmetic(FloatCoopVec4 x, FloatCoopVec4 y)
{
    return (x + y) * (x - y) - (x / y);
}

// Reuses the same CoopVec value on both sides of each arithmetic operation, guarding
// against AD paths dropping one of the two contributions.
[Differentiable]
FloatCoopVec4 testSameOperandArithmetic(FloatCoopVec4 x)
{
    return (x + x) + (x - x) + (x * x) + (x / x);
}

// Covers the element-wise binary and ternary derivative macros for CoopVec.
[Differentiable]
FloatCoopVec4 testMinMaxClampFma(FloatCoopVec4 x, FloatCoopVec4 y)
{
    let lo = FloatCoopVec4(1.5);
    let hi = FloatCoopVec4(4.5);
    return min(x, y) + max(x, y) + clamp(x, lo, hi) + fma(x, y, FloatCoopVec4(1.0));
}

// Exercises the value-pack CoopVec constructor path in reverse and forward AD.
[Differentiable]
FloatCoopVec4 testValuePackConstructor(float a, float b, float c, float d)
{
    return FloatCoopVec4(a, b, c, d) * FloatCoopVec4(2.0);
}

// Exercises the initializer-list CoopVec construction path, which lowers to MakeCoopVector.
[Differentiable]
FloatCoopVec4 testInitializerListConstructor(float a, float b, float c, float d)
{
    FloatCoopVec4 result = { a, b, c, d };
    return result * FloatCoopVec4(2.0);
}

// Exercises reverse-mode element updates when the assigned value has a `no_diff` attributed type.
[Differentiable]
FloatCoopVec4 testNoDiffElementUpdates(FloatCoopVec4 x, no_diff float value)
{
    FloatCoopVec4 result;
    for (int i = 0; i < 4; ++i)
        result[i] = value;
    return x + result;
}

// Exercises the CoopVec constructor from a vector value in reverse and forward AD.
[Differentiable]
FloatCoopVec4 testVectorConstructor(float4 value)
{
    return FloatCoopVec4(value) * FloatCoopVec4(2.0);
}

// Exercises the CoopVec constructor from an array value in reverse and forward AD.
[Differentiable]
FloatCoopVec4 testArrayConstructor(float a, float b, float c, float d)
{
    float values[4] = { a, b, c, d };
    return FloatCoopVec4(values) * FloatCoopVec4(2.0);
}

//TEST_INPUT:ubuffer(data=[0], stride=4, count=184):out,name=outputBuffer
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

    {
        // Forward-mode arithmetic checks both primal and tangent outputs.
        let x = FloatCoopVec4(2.0, 3.0, 4.0, 5.0);
        let y = FloatCoopVec4(1.0, 1.0, 1.0, 1.0);
        let xT = FloatCoopVec4(1.0, 2.0, 3.0, 4.0);
        let yT = FloatCoopVec4(1.0, 1.0, 1.0, 1.0);

        let result = fwd_diff(testForwardArithmetic)(diffPair(x, xT), diffPair(y, yT));

        result.p.store(outputBuffer, 160);
        result.d.store(outputBuffer, 176);
    }

    {
        // Same-operand reverse-mode arithmetic catches accidental loss of one input path.
        let x = FloatCoopVec4(2.0, 3.0, 4.0, 5.0);
        let dOut = FloatCoopVec4(1.0, 10.0, 100.0, 1000.0);

        let primal = testSameOperandArithmetic(x);
        var dx = DPFloatCoopVec4(x, FloatCoopVec4(0.0, 0.0, 0.0, 0.0));
        bwd_diff(testSameOperandArithmetic)(dx, dOut);

        primal.store(outputBuffer, 192);
        dx.d.store(outputBuffer, 208);
    }

    {
        // min/max/clamp/fma derivatives cover binary/ternary CoopVec AD macro expansions.
        let x = FloatCoopVec4(1.0, 2.0, 3.0, 4.0);
        let y = FloatCoopVec4(2.0, 1.0, 3.0, 5.0);
        let dOut = FloatCoopVec4(1.0, 10.0, 100.0, 1000.0);

        let primal = testMinMaxClampFma(x, y);
        var dx = DPFloatCoopVec4(x, FloatCoopVec4(0.0, 0.0, 0.0, 0.0));
        var dy = DPFloatCoopVec4(y, FloatCoopVec4(0.0, 0.0, 0.0, 0.0));
        bwd_diff(testMinMaxClampFma)(dx, dy, dOut);

        primal.store(outputBuffer, 224);
        dx.d.store(outputBuffer, 240);
        dy.d.store(outputBuffer, 256);
    }

    {
        // The value-pack constructor should transpose the CoopVec output gradient back
        // to the individual scalar constructor operands.
        let dOut = FloatCoopVec4(1.0, 10.0, 100.0, 1000.0);
        let primal = testValuePackConstructor(1.0, 2.0, 3.0, 4.0);
        var da = diffPair(1.0, 0.0);
        var db = diffPair(2.0, 0.0);
        var dc = diffPair(3.0, 0.0);
        var dd = diffPair(4.0, 0.0);
        bwd_diff(testValuePackConstructor)(da, db, dc, dd, dOut);

        primal.store(outputBuffer, 272);
        outputBuffer[72] = da.d;
        outputBuffer[73] = db.d;
        outputBuffer[74] = dc.d;
        outputBuffer[75] = dd.d;
    }

    {
        // Initializer-list construction should take the direct MakeCoopVector transpose path.
        let dOut = FloatCoopVec4(1.0, 10.0, 100.0, 1000.0);
        let primal = testInitializerListConstructor(1.0, 2.0, 3.0, 4.0);
        var da = diffPair(1.0, 0.0);
        var db = diffPair(2.0, 0.0);
        var dc = diffPair(3.0, 0.0);
        var dd = diffPair(4.0, 0.0);
        bwd_diff(testInitializerListConstructor)(da, db, dc, dd, dOut);

        primal.store(outputBuffer, 304);
        outputBuffer[80] = da.d;
        outputBuffer[81] = db.d;
        outputBuffer[82] = dc.d;
        outputBuffer[83] = dd.d;
    }

    {
        // Forward-mode mirror for compound arithmetic keeps both CoopVec operands active.
        let x = FloatCoopVec4(2.0, 3.0, 4.0, 5.0);
        let y = FloatCoopVec4(1.0, 2.0, 3.0, 4.0);
        let xT = FloatCoopVec4(1.0, 2.0, 3.0, 4.0);
        let yT = FloatCoopVec4(1.0, 1.0, 1.0, 1.0);

        let result = fwd_diff(testCompoundArithmetic)(diffPair(x, xT), diffPair(y, yT));

        result.p.store(outputBuffer, 336);
        result.d.store(outputBuffer, 352);
    }

    {
        // Forward-mode mirror for common math uses x = 0 for stable exact derivatives.
        let x = FloatCoopVec4(0.0, 0.0, 0.0, 0.0);
        let xT = FloatCoopVec4(1.0, 2.0, 3.0, 4.0);

        let result = fwd_diff(testCommonMath)(diffPair(x, xT));

        result.p.store(outputBuffer, 368);
        result.d.store(outputBuffer, 384);
    }

    {
        // Forward-mode mirror for division and negation keeps y = 1 to avoid noisy fractions.
        let x = FloatCoopVec4(2.0, 3.0, 4.0, 5.0);
        let y = FloatCoopVec4(1.0, 1.0, 1.0, 1.0);
        let xT = FloatCoopVec4(1.0, 2.0, 3.0, 4.0);
        let yT = FloatCoopVec4(1.0, 1.0, 1.0, 1.0);

        let result = fwd_diff(testDivisionNegation)(diffPair(x, xT), diffPair(y, yT));

        result.p.store(outputBuffer, 400);
        result.d.store(outputBuffer, 416);
    }

    {
        // Forward-mode mirror for the additional math intrinsics covers the same element-wise set.
        let x = FloatCoopVec4(0.0, 0.0, 0.0, 0.0);
        let xT = FloatCoopVec4(1.0, 2.0, 3.0, 4.0);

        let result = fwd_diff(testAdditionalMath)(diffPair(x, xT));

        result.p.store(outputBuffer, 432);
        result.d.store(outputBuffer, 448);
    }

    {
        // Forward-mode mirror for same-operand arithmetic checks shared primal use in JVP.
        let x = FloatCoopVec4(2.0, 3.0, 4.0, 5.0);
        let xT = FloatCoopVec4(1.0, 2.0, 3.0, 4.0);

        let result = fwd_diff(testSameOperandArithmetic)(diffPair(x, xT));

        result.p.store(outputBuffer, 464);
        result.d.store(outputBuffer, 480);
    }

    {
        // Forward-mode mirror for min/max/clamp/fma covers binary and ternary CoopVec derivatives.
        let x = FloatCoopVec4(1.0, 2.0, 3.0, 4.0);
        let y = FloatCoopVec4(2.0, 1.0, 3.0, 5.0);
        let xT = FloatCoopVec4(1.0, 2.0, 3.0, 4.0);
        let yT = FloatCoopVec4(1.0, 1.0, 1.0, 1.0);

        let result = fwd_diff(testMinMaxClampFma)(diffPair(x, xT), diffPair(y, yT));

        result.p.store(outputBuffer, 496);
        result.d.store(outputBuffer, 512);
    }

    {
        // Forward-mode mirror for the value-pack constructor links scalar tangents to CoopVec lanes.
        let result = fwd_diff(testValuePackConstructor)(
            diffPair(1.0, 1.0),
            diffPair(2.0, 10.0),
            diffPair(3.0, 100.0),
            diffPair(4.0, 1000.0));

        result.p.store(outputBuffer, 528);
        result.d.store(outputBuffer, 544);
    }

    {
        // Forward-mode mirror for initializer-list construction covers direct MakeCoopVector JVP.
        let result = fwd_diff(testInitializerListConstructor)(
            diffPair(1.0, 1.0),
            diffPair(2.0, 10.0),
            diffPair(3.0, 100.0),
            diffPair(4.0, 1000.0));

        result.p.store(outputBuffer, 560);
        result.d.store(outputBuffer, 576);
    }

    {
        // The no_diff value contributes no gradient, while the CoopVec input receives dOut.
        let x = FloatCoopVec4(2.0, 2.0, 2.0, 2.0);
        let dOut = FloatCoopVec4(1.0, 10.0, 100.0, 1000.0);
        var dx = DPFloatCoopVec4(x, FloatCoopVec4(0.0, 0.0, 0.0, 0.0));
        bwd_diff(testNoDiffElementUpdates)(dx, 3.0f, dOut);

        dx.d.store(outputBuffer, 592);
    }

    {
        // Vector construction should transpose the CoopVec output gradient back
        // to the source vector lanes.
        let value = float4(1.0, 2.0, 3.0, 4.0);
        let dOut = FloatCoopVec4(1.0, 10.0, 100.0, 1000.0);
        let primal = testVectorConstructor(value);
        var dValue = diffPair(value, float4(0.0, 0.0, 0.0, 0.0));
        bwd_diff(testVectorConstructor)(dValue, dOut);

        primal.store(outputBuffer, 608);
        outputBuffer[156] = dValue.d.x;
        outputBuffer[157] = dValue.d.y;
        outputBuffer[158] = dValue.d.z;
        outputBuffer[159] = dValue.d.w;
    }

    {
        // Array construction should transpose the CoopVec output gradient back
        // to the array elements.
        let dOut = FloatCoopVec4(1.0, 10.0, 100.0, 1000.0);
        let primal = testArrayConstructor(1.0, 2.0, 3.0, 4.0);
        var da = diffPair(1.0, 0.0);
        var db = diffPair(2.0, 0.0);
        var dc = diffPair(3.0, 0.0);
        var dd = diffPair(4.0, 0.0);
        bwd_diff(testArrayConstructor)(da, db, dc, dd, dOut);

        primal.store(outputBuffer, 640);
        outputBuffer[164] = da.d;
        outputBuffer[165] = db.d;
        outputBuffer[166] = dc.d;
        outputBuffer[167] = dd.d;
    }

    {
        // Forward-mode mirror for vector construction links vector tangents to
        // CoopVec lanes.
        let result = fwd_diff(testVectorConstructor)(
            diffPair(float4(1.0, 2.0, 3.0, 4.0), float4(1.0, 10.0, 100.0, 1000.0)));

        result.p.store(outputBuffer, 672);
        result.d.store(outputBuffer, 688);
    }

    {
        // Forward-mode mirror for array construction links scalar tangents to
        // CoopVec lanes through the temporary array.
        let result = fwd_diff(testArrayConstructor)(
            diffPair(1.0, 1.0),
            diffPair(2.0, 10.0),
            diffPair(3.0, 100.0),
            diffPair(4.0, 1000.0));

        result.p.store(outputBuffer, 704);
        result.d.store(outputBuffer, 720);
    }
}
