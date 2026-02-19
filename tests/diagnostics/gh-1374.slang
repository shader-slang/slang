// gh-1374.slang

// Test a `static` variable wwith a definition that refers to itself

//DIAGNOSTIC_TEST:SIMPLE(diag=diag):-stage compute -entry main -target hlsl

struct S
{
    static const int kVal = kVal;
//diag:              ^^^^ variable used in own initializer
//diag:              ^^^^ the initial-value expression for variable 'kVal' depends on the value of the variable itself

    static const int kInf = kInf + 1;
//diag:              ^^^^ variable used in own initializer
//diag:              ^^^^ the initial-value expression for variable 'kInf' depends on the value of the variable itself

    static const int kA = kB;
//diag:              ^^ variable used in own initializer
//diag:              ^^ the initial-value expression for variable 'kA' depends on the value of the variable itself
    static const int kB = kA;
//diag:              ^^ variable used in own initializer
//diag:              ^^ the initial-value expression for variable 'kB' depends on the value of the variable itself
}

[numthreads(1, 1, 1)]
void main(
    uint3 dispatchThreadID : SV_DispatchThreadID)
{
}
