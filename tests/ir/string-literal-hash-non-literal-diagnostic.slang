// Regression test: `getStringHash` on an argument that never folds to a string literal must
// report E41023 rather than crashing the compiler.
//
// `checkGetStringHashInsts` used to ask `IRGetStringHash::getStringLit()` whether operand 0
// was a literal. That accessor is a generated unchecked cast — `(IRStringLit*)getOperand(0)` —
// so for the run-time select below it returned a non-null pointer to an inst of a different
// type and the check never fired. `CLikeSourceEmitter::defaultEmitInstExpr` then called
// `getStringSlice()` on that same pointer and hashed whatever memory followed it, which
// segfaulted slangc instead of diagnosing the program.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<uint> outputBuffer;
uniform int sel;

[numthreads(1, 1, 1)]
void main()
{
    String s = sel > 0 ? "aaa" : "bbb";
    outputBuffer[0] = getStringHash(s);
//CHECK:                           ^ getStringHash can only be called when argument is statically resolvable to a string literal
//CHECK: E41023
}
