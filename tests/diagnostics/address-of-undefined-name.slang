//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// The operand of `__getAddress` that is an undefined name gets only the undefined-identifier error,
// not also error 31160 for taking the address of something that is not addressable. The test is
// exhaustive, so any extra diagnostic fails it.

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    let p = __getAddress(undefinedName);
//diag:                  ^^^^^^^^^^^^^ undefined identifier 'undefinedName'.
//diag:                  ^^^^^^^^^^^^^ undefined identifier
    outputBuffer[0] = 0;
}
