//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -experimental-feature

// Failed target inference must not synthesize a derivative interface conformance.
T id<T : IComparable>(T x)
{
    return x;
}

__func_extension<T : IDifferentiable & IComparable>
bwd_diff(id)(inout DifferentialPair<T> dpx, T.Differential dOut) -> void
//CHECK: expected a function
//CHECK: expected a function, got '(no_diff T) -> void'
{
    dpx = diffPair(dpx.p, dOut);
}
