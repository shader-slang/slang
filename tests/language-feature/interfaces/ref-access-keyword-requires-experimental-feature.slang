// The `__ref_readonly` and `__ref_writeonly` keywords are intended for the core module, so user
// code must enable `-experimental-feature` to use them. The legacy alias `const __ref` remains
// available without it.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute

int readOne(__ref_readonly int x) { return x; }
//CHECK:    ^^^^^^^^^^^^^^ E30133
//CHECK:    ^^^^^^^^^^^^^^ '__ref_readonly' is intended for the core module, so it is rejected in user code unless '-experimental-feature' is enabled.

void setOne(__ref_writeonly int x) { x = 1; }
//CHECK:    ^^^^^^^^^^^^^^^ E30133
//CHECK:    ^^^^^^^^^^^^^^^ '__ref_writeonly' is intended for the core module, so it is rejected in user code unless '-experimental-feature' is enabled.

int readLegacy(const __ref int x) { return x; }

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    int a = 0;
    setOne(a);
    outputBuffer[0] = readOne(a) + readLegacy(a);
}
