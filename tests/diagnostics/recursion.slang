//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -emit-spirv-directly

float test<let x : int>()
{
    return test<x>();
//CHECK:          ^ recursion not allowed
//CHECK:          ^ recursion detected in call to 'test', but the current code generation target does not allow recursion.
}

RWStructuredBuffer<float> outputBuffer;
[numthreads(1,1,1)]
void main()
{
    outputBuffer[0] = test<1>();
}