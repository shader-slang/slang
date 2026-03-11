//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int func<L>()
{
    int a = sizeof(float3, L);
//CHECK:                   ^ invalid sizeof data layout
//CHECK:                   ^ argument does not conform to IBufferDataLayout
    int b = sizeof(float3, 14);
//CHECK:                   ^^ invalid sizeof data layout
//CHECK:                   ^^ argument does not conform to IBufferDataLayout
    return a + b;
}

