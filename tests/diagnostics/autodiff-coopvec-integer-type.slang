//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Floating-point CoopVec types are differentiable, but integer CoopVec types should not be.

// Returning an integer CoopVec from a differentiable function should fail once AD tries to
// form the integer CoopVec differential pair.
[Differentiable]
CoopVec<int, 4> intReturn(CoopVec<float, 4> x)
{
    return CoopVec<int, 4>(1);
}

// Passing an integer CoopVec as a differentiable input should fail before AD translation.
[Differentiable]
CoopVec<float, 4> intParam(CoopVec<int, 4> x)
{
    return CoopVec<float, 4>(1.0);
}

// Reverse-mode should reject unsigned integer CoopVec inputs for the same reason.
[BackwardDifferentiable]
float uintParam(CoopVec<uint, 4> x, float y)
{
    return y * y;
}

void useIntReturn(CoopVec<float, 4> x)
{
    let pair = diffPair(x, CoopVec<float, 4>(1.0));
    let result: DifferentialPair<CoopVec<int, 4>> = fwd_diff(intReturn)(pair);
//CHECK:        ^^^^^^^^^^^^^^^^ type argument doesn't conform to interface
//CHECK:        ^^^^^^^^^^^^^^^^ type argument 'CoopVec<int, 4>' does not conform to the required interface 'IDifferentiable'
}

void useIntParam()
{
    let pair = DifferentialPair<CoopVec<int, 4>>(CoopVec<int, 4>(1), CoopVec<int, 4>(0));
//CHECK:       ^^^^^^^^^^^^^^^^ type argument doesn't conform to interface
//CHECK:       ^^^^^^^^^^^^^^^^ type argument 'CoopVec<int, 4>' does not conform to the required interface 'IDifferentiable'
    let result = fwd_diff(intParam)(pair);
}

void useUIntParam(float y)
{
    var xPair = DifferentialPair<CoopVec<uint, 4>>(CoopVec<uint, 4>(1), CoopVec<uint, 4>(0));
//CHECK:        ^^^^^^^^^^^^^^^^ type argument doesn't conform to interface
//CHECK:        ^^^^^^^^^^^^^^^^ type argument 'CoopVec<uint, 4>' does not conform to the required interface 'IDifferentiable'
    var yPair = diffPair(y, 0.0);
    bwd_diff(uintParam)(xPair, yPair, 1.0);
}
