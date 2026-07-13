//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct MissingDZero : IDifferentiable
{
    typealias Differential = MissingDZero;
    float val;

    static MissingDZero dzero()
//CHECK:                ^^^^^ callable differentiability requirement not satisfied
//CHECK:                ^^^^^ member 'dzero' does not satisfy the required forward-differentiable constraint for interface requirement 'dzero'.
    {
        return {0.0};
    }

    [Differentiable]
    static MissingDZero dadd(MissingDZero a, MissingDZero b)
    {
        return {a.val + b.val};
    }
};

struct MissingDAdd : IDifferentiable
{
    typealias Differential = MissingDAdd;
    float val;

    [Differentiable]
    static MissingDAdd dzero()
    {
        return {0.0};
    }

    static MissingDAdd dadd(MissingDAdd a, MissingDAdd b)
//CHECK:               ^^^^ callable differentiability requirement not satisfied
//CHECK:               ^^^^ member 'dadd' does not satisfy the required forward-differentiable constraint for interface requirement 'dadd'.
    {
        return {a.val + b.val};
    }
};

interface IForwardRequirement
{
    [ForwardDifferentiable]
    static float eval(This value, float x);
}

// A hard forward-differentiable static interface requirement should reject a
// synthesized wrapper that forwards to a non-differentiable non-static witness.
struct MissingForwardDerivative : IForwardRequirement
{
    float eval(float x)
//CHECK:  ^^^^ interface requirement mismatch
//CHECK:  ^^^^ member 'eval' does not match interface requirement.
//CHECK:  ^^^^ member 'eval' cannot satisfy differentiable interface requirement 'eval' because it does not provide the required differentiability; it may be missing an appropriate differentiability attribute, such as [Differentiable]
    {
        return x * x;
    }
};
