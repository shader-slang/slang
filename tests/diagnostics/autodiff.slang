//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

//CHECK: tests/diagnostics/autodiff.slang({{..}}): error 38031:
//CHECK-NEXT: float x1 = no_diff x;

//CHECK: tests/diagnostics/autodiff.slang({{..}}): error 38032:
//CHECK-NEXT: return no_diff f(x);

//CHECK: tests/diagnostics/autodiff.slang({{..}}): error 38033: 
//CHECK-NEXT: return no_diff nonDiff(x);

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
    return no_diff f(x);  // no_diff on a differentiable call has no meaning.
}

float n(float x)
{
    return no_diff nonDiff(x); // no_diff in a non-differentiable function
}