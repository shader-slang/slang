//DIAGNOSTIC_TEST:SIMPLE:

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
    return obj.fp.f;
}

[BackwardDifferentiable]
float h(float x)
{
    NoDiffField obj;
    obj.fp.f = detach(x * x); // OK.
    return obj.fp.f;
}
