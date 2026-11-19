// BFloat16 is a 16-bit floating-point element type, so a storage buffer
// holding it must declare the 16-bit storage capability. No explicit
// -capability flag is needed: the BFloat16 type's own capability requirement
// enables spvBFloat16KHR (emitted as BFloat16TypeKHR), and lowering
// RWStructuredBuffer<BFloat16> produces a StorageBuffer-class pointer whose
// 16-bit element selects UniformAndStorageBuffer16BitAccess. Unlike the
// sibling capability tests, no -profile is pinned here: the default SPIR-V
// version keeps the modern StorageBuffer encoding this test targets, while
// -profile spirv_1_3 selects the legacy uniform-block encoding.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute

RWStructuredBuffer<BFloat16> dst;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    dst[tid.x] = BFloat16(float(tid.x));
}

// A BFloat16 classified as 8-bit fails the required 16-bit DAG match below.
// The NOT pair is belt-and-braces for the both-flags case: capabilities print
// in insertion order and the 8-bit request is considered first, so an 8-bit
// line lands before the first DAG match, inside the leading NOT's region
// (module start to first match); the trailing NOT covers last match to end.
// CHECK-NOT: OpCapability UniformAndStorageBuffer8BitAccess
// CHECK-DAG: OpCapability UniformAndStorageBuffer16BitAccess
// CHECK-DAG: OpCapability BFloat16TypeKHR
// CHECK-NOT: OpCapability UniformAndStorageBuffer8BitAccess
