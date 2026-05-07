// pointer-cannot-take-local-address.slang

// Test that forming a pointer to a local variable is diagnosed as an error on GPU targets.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-stage compute -entry computeMain -target spirv

RWStructuredBuffer<int> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    int local = 5;
    int* p = __getAddress(local);
//CHECK:     ^^^^^^^^^^^^ invalid __getAddress usage
//CHECK:     ^^^^^^^^^^^^ cannot take the address of a function-local variable

    output[0] = *p;
}
