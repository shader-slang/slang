//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int func<L>()
{
    int a = sizeof(float3, L);
//CHECK:                   ^ invalid sizeof data layout
//CHECK:                   ^ 'L' does not conform to IBufferDataLayout
    int b = sizeof(float3, int);
//CHECK:                   ^^^ invalid sizeof data layout
//CHECK:                   ^^^ 'int' does not conform to IBufferDataLayout
    return a + b;
}

