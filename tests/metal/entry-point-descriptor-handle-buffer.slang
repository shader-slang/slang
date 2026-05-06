// Regression for #11066: Metal codegen omitted `[[buffer(n)]]` on
// `RWStructuredBuffer<T>.Handle` entry-point parameters while emitting it
// for plain `RWStructuredBuffer<T>`, so Metal's declaration-order implicit
// slots diverged from Slang reflection and cross-buffer corruption followed.
//
// Verify that every buffer-kind entry-point parameter -- including
// DescriptorHandle-wrapped buffers -- receives an explicit `[[buffer(n)]]`
// attribute on the Metal target.

//TEST:SIMPLE(filecheck=CHECK): -target metal -entry computeMain -stage compute

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

// All three buffers must get an explicit `[[buffer(N)]]` -- previously the
// two Handle parameters fell through without any attribute.  Slang emits
// the entry-point parameters in the KernelContext struct order:
// outputBuffer, biasBuffer, dataBuffer.  FileCheck's `[[...]]` is variable
// syntax, so the attribute brackets are matched with `{{\[\[}}`.
// CHECK: outputBuffer{{[_0-9]*}} {{\[\[}}buffer(
// CHECK: biasBuffer{{[_0-9]*}} {{\[\[}}buffer(
// CHECK: dataBuffer{{[_0-9]*}} {{\[\[}}buffer(
