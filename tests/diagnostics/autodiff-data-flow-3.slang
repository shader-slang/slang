//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct DiffT : IDifferentiable
{
    float f;
}
struct NoDiffField
{
    DiffT fp;
}

[BackwardDifferentiable]
float g(float x)
{
    NoDiffField obj;
    obj.fp.f = x * x; // Error, this location cannot hold derivative.
//CHECK:     ^ derivative is lost during assignment
//CHECK:     ^ derivative is lost during assignment to non-differentiable location, use 'detach()' to clarify intention.
    return obj.fp.f;
}

[BackwardDifferentiable]
void diffOut(inout float x)
{
    x = 2;
}

float noDiffFunc(float x)
{
    return 0.0;
}

[BackwardDifferentiable]
float h(float x)
{
    NoDiffField obj;
    obj.fp.f = detach(x * x); // OK.
    obj.fp.f = noDiffFunc(x); // OK.
    diffOut(obj.fp.f); // Error.
//CHECK:           ^ derivative is lost passing non-differentiable location
//CHECK:           ^ derivative is lost when passing a non-differentiable location to an `out` or `inout` parameter, consider passing a temporary variable instead.
    return obj.fp.f;
}
