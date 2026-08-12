//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Calling a generic function with more value arguments than its parameter list
// can match fails argument-to-parameter matching before inference runs, and
// should report a focused arity mismatch rather than the generic fallback.

void f<T>(T x) {}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    f(1, 2);
/*CHECK:
     ^ wrong number of arguments in call to generic function
     ^ could not specialize generic: it expects 1 parameter(s) but the call provides 2 argument(s)
*/
}
