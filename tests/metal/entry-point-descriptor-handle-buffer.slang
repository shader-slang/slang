// Regression for #11066: Handle-wrapped buffer params were missing
// [[buffer(n)]], so Metal's implicit declaration-order slots diverged from
// Slang reflection and writes landed in the wrong buffer.

//TEST:SIMPLE(filecheck=CHECK): -target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry computeMain -stage compute

uniform RWStructuredBuffer<float>.Handle dataBuffer;
uniform RWStructuredBuffer<float>.Handle biasBuffer;
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(uint tid : SV_GroupIndex)
{
    outputBuffer[0] = 1;
    if (tid == 0 && biasBuffer[0] == 12345.0f)
        outputBuffer[0] = 9;
    dataBuffer[64] = 0.125f;
}

// Emit order follows the entry-point KernelContext struct.
// CHECK: outputBuffer{{[_0-9]*}} {{\[\[}}buffer(
// CHECK: biasBuffer{{[_0-9]*}} {{\[\[}}buffer(
// CHECK: dataBuffer{{[_0-9]*}} {{\[\[}}buffer(

// Metal's compiler must accept the generated MSL end-to-end.
// METALLIB: @computeMain
