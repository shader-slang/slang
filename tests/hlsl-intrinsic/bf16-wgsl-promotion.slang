// WGSL does not natively support BFloat16 but the compiler gracefully
// promotes bf16 arithmetic to float. Verify the WGSL output is valid.

//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry computeMain

RWStructuredBuffer<float> outputBuffer;

// CHECK: fn computeMain
// CHECK: f32(
[numthreads(1,1,1)]
void computeMain()
{
    // Buffer input prevents constant folding, exercises the promotion path.
    BFloat16 a = BFloat16(outputBuffer[0]);
    BFloat16 b = BFloat16(outputBuffer[1]);
    BFloat16 c = a + b;
    outputBuffer[0] = float(c);
}
