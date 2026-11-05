//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -preserve-params -O0

// A shader whose only byte-address buffer is an unused `RasterizerOrderedByteAddressBuffer` still
// runs byte-address buffer legalization, so that `-preserve-params` hands the SPIR-V emitter a
// structured buffer it can declare (issue 14039).

RasterizerOrderedByteAddressBuffer unusedBuffer;

// CHECK: OpDecorate %unusedBuffer Binding 0
// CHECK: %unusedBuffer = OpVariable

[numthreads(1, 1, 1)]
void computeMain() {}
