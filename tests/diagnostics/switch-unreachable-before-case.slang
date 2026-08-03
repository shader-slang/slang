//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

// A statement before the first case/default in a switch is unreachable and
// must warn (E41000); a multi-statement leading run warns once.

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    switch (int(tid.x))
    {
        outputBuffer[0] = 99;
//CHECK:^^^^^^^^^^^^ E41000
        outputBuffer[0] = 88;

    case 0:
        outputBuffer[0] = 1;
        break;
        outputBuffer[0] = 42;
//CHECK:^^^^^^^^^^^^ E41000

    case 1:
        outputBuffer[0] = 2;
        break;
    }
}
