// WGSL does not natively support BFloat16. Verify the compiler rejects
// BFloat16 usage on the WGSL target with a clear error.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry computeMain

RWStructuredBuffer<float> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    BFloat16 a = BFloat16(outputBuffer[0]);
    BFloat16 b = BFloat16(outputBuffer[1]);
    BFloat16 c = a + b;
    outputBuffer[0] = float(c);
}

// CHECK: BFloat16
