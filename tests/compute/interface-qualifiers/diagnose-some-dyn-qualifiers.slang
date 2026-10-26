// The `dyn` operator forms a box return type, but its operand must name an interface.
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

// A box return type is valid on an ordinary function declaration.
dyn IFoo globalFunc() { return A(); }

// Struct and enum names do not denote interfaces.
dyn A badStructValue;
/*CHECK:
    ^ expected an interface or interface conjunction, got 'A'
*/

enum BadEnum
{
    Val1,
    Val2
}
dyn BadEnum badEnumValue;
/*CHECK:
    ^^^^^^^ expected an interface or interface conjunction, got 'BadEnum'
*/

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 0;
}
