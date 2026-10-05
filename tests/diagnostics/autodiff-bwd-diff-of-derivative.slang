//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// `bwd_diff` applied directly to a generated derivative is rejected; a function that uses
// `fwd_diff` can be differentiated in backward mode instead.

[Differentiable]
float cube(float t) { return t * t * t; }

[Differentiable]
T genericCube<T : __BuiltinFloatingPointType>(T t) { return t * t * t; }

[TreatAsDifferentiable]
float trivialCube(float t) { return t * t * t; }

[BackwardDerivative(customCubeBwd)]
float customCube(float t) { return t * t * t; }

void customCubeBwd(inout DifferentialPair<float> t, float seed)
{
    t = diffPair(t.p, 3.0 * t.p * t.p * seed);
}

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

    __bwd_diff(__fwd_diff(genericCube<float>))(dp, diffPair(0.0, 1.0));
//CHECK:       ^^^^^^^^^^ cannot apply 'bwd_diff' directly to a generated derivative
//CHECK:       ^^^^^^^^^^ 'bwd_diff' of a generated derivative is not supported; apply 'bwd_diff' to a function that uses 'fwd_diff' instead.

    __bwd_diff(__fwd_diff(trivialCube))(dp, diffPair(0.0, 1.0));
//CHECK:       ^^^^^^^^^^ cannot apply 'bwd_diff' directly to a generated derivative
//CHECK:       ^^^^^^^^^^ 'bwd_diff' of a generated derivative is not supported; apply 'bwd_diff' to a function that uses 'fwd_diff' instead.

    var seed = diffPair(1.0);
    __bwd_diff(__bwd_diff(cube))(dp, seed);
/*CHECK:
    ^^^^^^^^^^ member not found
    ^^^^^^^^^^ 'bwd_diff' is not a member of 'cube.bwd_diff'.
*/

    // A FunctionCopy wrapper for a custom backward derivative uses ordinary lookup too.
    __bwd_diff(__bwd_diff(customCube))(dp, seed);
/*CHECK:
    ^^^^^^^^^^ member not found
    ^^^^^^^^^^ 'bwd_diff' is not a member of 'customCube.bwd_diff'.
*/
}
