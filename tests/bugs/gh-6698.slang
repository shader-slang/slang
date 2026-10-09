// Test for issue #6698: Passing an unsized array to a function that transitively
// calls getCount() should produce a diagnostic (E56002).
//
// The check works for entry-point-reachable code: when an unsized array is
// passed through a call chain that reaches getCount(), E56002 fires. The unsized
// array lives in a storage buffer, because a global uniform `int a[]` is itself
// an error (E31215).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

int f(int b[]) { return b.getCount(); }
//CHECK:                          ^ cannot get size of unsized array
//CHECK:                          ^ cannot obtain the size of an unsized array.
int g(int b[]) { return f(b); }

struct Data
{
    int values[];
}
GLSLShaderStorageBuffer<Data> buffer;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = g(buffer.values); // Not OK: unsized param reaches getCount
}
