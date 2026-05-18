//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct MyFloat : IDifferentiable
{
    typealias Differential = MyFloat;
    float val;

    static MyFloat dzero()
//CHECK:           ^^^^^ interface requirement mismatch
//CHECK:           ^^^^^ member 'dzero' does not match interface requirement.
//CHECK:           ^^^^^ member 'dzero' cannot satisfy differentiable interface requirement 'dzero'
    {
        return { 0.0 };
    }

    static MyFloat dadd(MyFloat a, MyFloat b)
//CHECK:           ^^^^ interface requirement mismatch
//CHECK:           ^^^^ member 'dadd' does not match interface requirement.
//CHECK:           ^^^^ member 'dadd' cannot satisfy differentiable interface requirement 'dadd'
    {
        return { a.val + b.val };
    }
};
