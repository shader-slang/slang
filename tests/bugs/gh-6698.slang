// Test for issue #6698: Passing an unsized array to a function that transitively
// calls getCount() should produce a diagnostic (E56002).
//
// The check works for entry-point-reachable code: when an unsized array is
// passed through a call chain that reaches getCount(), E56002 fires.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

int f(int b[]) { return b.getCount(); }
//CHECK:                          ^ cannot get size of unsized array
//CHECK:                          ^ cannot obtain the size of an unsized array.
int g(int b[]) { return f(b); }

uniform int unsizedParam[];

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    g(unsizedParam); // Not OK: unsized param reaches getCount
}
