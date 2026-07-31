//TEST:SIMPLE: -target spirv -entry computeMain -stage compute -o bwd-derivative-of-explicit-overloaded-member.spv

// A derivative method has an implicit receiver, while explicitly referencing its target as
// `Type::method` makes the differentiated callable expose that receiver as parameter zero.
// Overload resolution must use the derivative method's receiver before selecting by the declared
// parameters.

struct Scale
{
    float factor;

    [Differentiable]
    [NoDiffThis]
    float apply(float value)
    {
        return factor * value;
    }

    [Differentiable]
    static float apply(float left, float right)
    {
        return left + right;
    }

    [BackwardDerivativeOf(Scale::apply)]
    [NoDiffThis]
    void applyBackward(inout DifferentialPair<float> value, float resultGradient)
    {
        value = diffPair(value.p, factor * resultGradient);
    }

    [BackwardDerivativeOf(Scale::apply)]
    static void applyStaticBackward(
        inout DifferentialPair<float> left,
        inout DifferentialPair<float> right,
        float resultGradient)
    {
        left = diffPair(left.p, resultGradient);
        right = diffPair(right.p, resultGradient);
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{}
