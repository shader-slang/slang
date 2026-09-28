//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute -disable-non-essential-validations

// `-disable-non-essential-validations` skips `checkGetStringHashInsts`, so the
// emitter's own test of the operand is all that stands between a non-literal
// `getStringHash` and reading another instruction's storage as string data. It
// cannot report E41023 -- that diagnostic belongs to the check that was skipped
// -- but it must reach its unhandled-instruction path rather than hashing
// whatever the operand happens to be.

RWStructuredBuffer<uint> outputBuffer;
uniform int sel;

[numthreads(1, 1, 1)]
void main()
{
    String s = sel > 0 ? "aaa" : "bbb";
    outputBuffer[0] = getStringHash(s);
//CHECK: E99999
}
