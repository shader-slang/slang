//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct MyFloat : IDifferentiable
{
    typealias Differential = MyFloat;
    float val;

    static MyFloat dzero()
//CHECK:           ^^^^^ interface requirement mismatch
//CHECK:           ^^^^^ member 'dzero' does not match interface requirement.
//CHECK:           ^^^^^ may be missing an appropriate differentiability attribute, such as [Differentiable]
    {
        return { 0.0 };
    }

    static MyFloat dadd(MyFloat a, MyFloat b)
//CHECK:           ^^^^ interface requirement mismatch
//CHECK:           ^^^^ member 'dadd' does not match interface requirement.
//CHECK:           ^^^^ may be missing an appropriate differentiability attribute, such as [Differentiable]
    {
        return { a.val + b.val };
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
//CHECK:  ^^^^ may be missing an appropriate differentiability attribute, such as [Differentiable]
    {
        return x * x;
    }
};
