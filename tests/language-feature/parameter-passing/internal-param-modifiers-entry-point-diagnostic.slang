// An entry-point parameter using a new mode is rejected before the entry-point
// uniform-parameter collection (which would otherwise erase the param-mode wrapper
// and let the unsupported mode reach code generation silently on some targets).
// This is the entry-point counterpart of the ordinary-function reject in
// internal-param-modifiers-diagnostic.slang.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(__ref_readonly int x)
{
/*CHECK: ([[# @LINE-2]]): error 56006
*/
    outputBuffer[0] = x;
}
