//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -stage compute -entry computeMain

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float f = (float)tid.x;
    switch (f)
//CHECK:    ^ error E30607
//CHECK:    ^ 'switch' condition must be of an integer or enum type, but is of type 'float'
    {
    case 1:
        outputBuffer[0] = 1;
        break;
    default:
        outputBuffer[0] = 0;
        break;
    }
}
