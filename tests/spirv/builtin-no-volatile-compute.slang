// builtin-no-volatile-compute.slang
//
// Regression test for shader-slang/slang#10528.
// `Volatile` on subgroup builtins is a raytracing-stage requirement;
// compute shaders must not get the extra decoration.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -capability vk_mem_model

RWStructuredBuffer<uint> output;

// Single-entry-point shader; no use of `Volatile` should appear in
// the SPIR-V at all under either memory model. The leading
// `CHECK-NOT: Volatile` scans file start → first `CHECK` match; the
// trailing one scans last `CHECK` match → EOF. Together they cover
// the whole disassembly, rejecting both the variable-level
// decoration (GLSL450 path) and the per-`OpLoad` mask
// (vk_mem_model path).
// CHECK-NOT: Volatile
// CHECK: OpDecorate %{{[A-Za-z0-9_]+}} BuiltIn SubgroupLocalInvocationId
// CHECK-NOT: Volatile
[numthreads(64, 1, 1)]
[shader("compute")]
void computeMain()
{
    output[0] = WaveGetLaneIndex();
}
