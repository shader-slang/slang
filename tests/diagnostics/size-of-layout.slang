//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int func<L>()
{
    int a = sizeof(float3, L);
//CHECK:                   ^ invalid data layout for sizeof/alignof
//CHECK:                   ^ argument does not conform to IBufferDataLayout
    int b = sizeof(float3, 14);
//CHECK:                   ^^ invalid data layout for sizeof/alignof
//CHECK:                   ^^ argument does not conform to IBufferDataLayout
    return a + b;
}

