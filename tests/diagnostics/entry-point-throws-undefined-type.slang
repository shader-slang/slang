//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute
//
// Companion to entry-point-cannot-throw.slang (issue #12330).
//
// When the `throws` clause names a type that does not exist, the type check reports that and
// leaves the error type as the error sentinel rather than the bottom type. The entry-point
// check has to treat that sentinel as "nothing was declared", or it reports a second failure
// on top of the one the user actually has to fix. Only the undefined-identifier error belongs
// here.

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID) throws NotAType
/*CHECK:
                                                         ^^^^^^^^ undefined identifier
                                                         ^^^^^^^^ undefined identifier 'NotAType'.
*/
{
    outputBuffer[tid.x] = tid.x;
}
