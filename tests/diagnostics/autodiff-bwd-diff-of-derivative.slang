//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// `bwd_diff` applied directly to a generated derivative is rejected; a function that uses
// `fwd_diff` can be differentiated in backward mode instead.

[Differentiable]
float cube(float t) { return t * t * t; }

struct Cubic
{
    [Differentiable]
    [NoDiffThis]
    float eval(float t) { return t * t * t; }
}

void test()
{
    var dp = diffPair(diffPair(1.0, 1.0), diffPair(0.0, 0.0));
    __bwd_diff(__fwd_diff(cube))(dp, diffPair(0.0, 1.0));
//CHECK:       ^^^^^^^^^^ cannot apply 'bwd_diff' directly to a generated derivative
//CHECK:       ^^^^^^^^^^ 'bwd_diff' of a generated derivative is not supported; apply 'bwd_diff' to a function that uses 'fwd_diff' instead.

    Cubic c;
    __bwd_diff(__fwd_diff(c.eval))(dp, diffPair(0.0, 1.0));
//CHECK:       ^^^^^^^^^^ cannot apply 'bwd_diff' directly to a generated derivative
//CHECK:       ^^^^^^^^^^ 'bwd_diff' of a generated derivative is not supported; apply 'bwd_diff' to a function that uses 'fwd_diff' instead.
}
