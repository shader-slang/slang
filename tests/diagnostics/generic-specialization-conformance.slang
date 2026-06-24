//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// A generic function constrained by `T : IFoo` cannot be specialized when the
// inferred argument type does not conform to the required interface. The
// required subtype witness cannot be proven during generic argument solving, so
// the call should report a focused non-conformance diagnostic naming the
// offending argument type and interface rather than the generic fallback.

interface IFoo
{
    int get();
}

void f<T : IFoo>(T x) {}

struct Bar {}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    Bar b;
    f(b);
/*CHECK:
     ^ type argument doesn't conform to interface
     ^ type argument 'Bar' does not conform to the required interface 'IFoo'
*/
}
