// Atomic-counter legalization currently supports directly-bound globals, not arrays. Keep a
// binding on this declaration so the regression proves that the aggregate shape itself is
// rejected before its placeholder element type can reach direct SPIR-V emission.

//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang glsl -Wno-118 -target spirv -emit-spirv-directly -entry main -stage compute tests/diagnostics/glsl-atomic-counter-array-not-supported.slang

layout(binding = 0, offset = 0) uniform atomic_uint counters[4];
//CHECK: arrays of `atomic_uint` are not supported
//CHECK: declare each supported atomic counter as a directly-bound global

layout(local_size_x = 1) in;

void main()
{
    uint value = atomicCounterIncrement(counters[0]);
}
