//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -profile spirv_1_0

// Regression test for shader-slang/slang#11631.
//
// An entry-point `[require(spirv_1_x)]` attribute must raise the emitted SPIR-V binary
// version the same way `-capability spirv_1_x` / `-profile spirv_1_x` does.
// `[require(spirv_1_5)]` lowers to an IRRequireCapabilityAtomDecoration carrying the
// internal `_spirv_1_5` atom; determineSpirvVersion() previously scanned that decoration
// but matched only the public `spirv_1_5` alias, so the requirement was silently dropped
// and the header stayed at the `-profile spirv_1_0` floor.
//
// This is the exact repro from the issue: compiled at `-profile spirv_1_0`, the
// `[require(spirv_1_5)]` must bump the emitted header to 1.5.

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
