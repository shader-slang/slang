//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -profile spirv_1_0

// Regression test for shader-slang/slang#11631.
//
// An entry-point `[require(spirv_1_5)]` must raise the emitted SPIR-V binary version the same way
// `-capability spirv_1_5` / `-profile spirv_1_5` does: compiled at `-profile spirv_1_0`, the
// header must be 1.5, not the profile floor of 1.0.

// CHECK: Version: 1.5

[[vk::binding(0)]]
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
[require(spirv_1_5)]
void computeMain()
{
    outputBuffer[0] = 1;
}
