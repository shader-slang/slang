//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature

// Failed target inference must not synthesize a derivative interface conformance.
T id<T : IComparable>(T x)
{
    return x;
}

__func_extension<T : IDifferentiable & IComparable>
fwd_diff(id)(DifferentialPair<T> dpx) -> DifferentialPair<T>
//CHECK: expected a function
//CHECK: expected a function, got '(T) -> T'
{
    return dpx;
}
