// A concrete divide-by-zero in a constant context (here an array extent) folds via
// BuiltinOperationIntVal::tryFoldImpl. The diagnostic must point at the offending operator -- it
// previously had no source location because the decl-free BuiltinOperationIntVal had nowhere to
// take a location from; tryConstantFoldBuiltinOperatorExpr now passes the operator expr's loc.
//DIAGNOSTIC_TEST:SIMPLE(diag=DIAG):-target hlsl -stage compute -entry main

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    int a[1 / 0];
/*DIAG:
            ^ divide by zero
*/
    outputBuffer[0] = a.getCount();
}
