// Diagnostic tests for `dyn` qualifier in invalid positions.
//
// The `some` qualifier is not yet implemented (SP#024, tracked by #7115),
// so `some`-specific diagnostics cannot be tested yet.
//
// Each E31201 position emits both a short summary and a detailed message.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -lang slang -std 2025 -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -lang slang -std 2025 -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda -lang slang -std 2025 -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target metal -lang slang -std 2025 -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target wgsl -lang slang -std 2025 -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cpp -lang slang -std 2025 -stage compute -entry computeMain

interface IFoo
{
    int get();
}

struct A : IFoo
{
    int get() { return 1; }
}

// `dyn` on a function declaration (parsed as declaration modifier, not return type qualifier) is rejected.
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
