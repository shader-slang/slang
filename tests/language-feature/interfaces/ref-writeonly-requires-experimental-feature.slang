// Reads of a `__ref_writeonly` parameter are not yet diagnosed, so user code must enable
// `-experimental-feature` to declare one.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute

void setOne(__ref_writeonly int x) { x = 1; }
//CHECK:    ^^^^^^^^^^^^^^^ E30133
//CHECK:    ^^^^^^^^^^^^^^^ reads of a '__ref_writeonly' parameter are not yet diagnosed, so it is rejected unless '-experimental-feature' is enabled.

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    int a = 0;
    setOne(a);
    outputBuffer[0] = a;
}
