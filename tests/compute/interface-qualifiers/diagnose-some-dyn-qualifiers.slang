// Diagnostic tests for `dyn` qualifier in invalid positions.
//
// The `some` qualifier is not yet implemented (SP#024, tracked by #7115),
// so `some`-specific diagnostics cannot be tested yet.
//
// Each E31201 position emits both a short summary and a detailed message.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target metal -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target wgsl -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cpp -stage compute -entry computeMain

interface IFoo
{
    int get();
}

struct A : IFoo
{
    int get() { return 1; }
}

// `dyn` as function modifier is rejected.
dyn IFoo globalFunc() { return A(); }
/*CHECK:
^^^ modifier not allowed
^^^ modifier 'dyn' is not allowed here.
*/

// `dyn` on struct declarations is rejected.
dyn struct BadStruct
/*CHECK:
^^^ modifier not allowed
^^^ modifier 'dyn' is not allowed here.
*/
{
    int x;
}

// `dyn` on enum declarations is rejected.
dyn enum BadEnum
/*CHECK:
^^^ modifier not allowed
^^^ modifier 'dyn' is not allowed here.
*/
{
    Val1,
    Val2
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 0;
}
