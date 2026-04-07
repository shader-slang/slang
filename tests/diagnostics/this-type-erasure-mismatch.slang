//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

interface ISelfOp
{
    This clone();
    bool matches(This other);
}

struct Impl : ISelfOp
{
    float v;
    This clone() { return Impl(v); }
    bool matches(Impl other) { return v == other.v; }
}

void test()
{
    ISelfOp obj = Impl(5.0);
    ISelfOp copy = obj.clone();
    obj.matches(copy);
    //CHECK: type mismatch in expression
    //CHECK: expected an expression of type 'test..This', got 'test..This'
    //CHECK: the concrete 'This' type identity is lost after type erasure
}
