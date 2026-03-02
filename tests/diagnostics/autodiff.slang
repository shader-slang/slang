//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

float nonDiff(float x)
{
    return x;
}

[ForwardDifferentiable]
float f(float x)
{
    float val = 0;
    if (x > 5)
        val = x + 1;

    [MaxIters(2)]
    for (int i = 0; i < (int)x; i++) // OK
    {
    }

    for (int i = 0; i < 5; i++) // OK
    {
    }

    return val;
}

[ForwardDifferentiable]
float m(float x)
{
    float x1 = no_diff x; // invalid use of no_diff here.
//CHECK:       ^^^^^^^ invalid 'no_diff' usage
//CHECK:       ^^^^^^^ 'no_diff' can only be used to decorate a call or a subscript operation
    return no_diff f(x);  // no_diff on a differentiable call has no meaning.
//CHECK:   ^^^^^^^ 'no_diff' on differentiable function has no meaning
//CHECK:   ^^^^^^^ use 'no_diff' on a call to a differentiable function has no meaning.
}

float n(float x)
{
    return no_diff nonDiff(x); // no_diff in a non-differentiable function
//CHECK:   ^^^^^^^ 'no_diff' in non-differentiable function
//CHECK:   ^^^^^^^ cannot use 'no_diff' in a non-differentiable function.
}
