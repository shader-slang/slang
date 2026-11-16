// BFloat16 is a 16-bit floating-point element type, so a storage buffer
// holding it must declare the 16-bit storage capability. No explicit
// -capability flag is needed: the BFloat16 type's own capability requirement
// enables spvBFloat16KHR, and lowering RWStructuredBuffer<BFloat16> produces
// a StorageBuffer-class pointer whose 16-bit element selects
// UniformAndStorageBuffer16BitAccess.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute

RWStructuredBuffer<BFloat16> dst;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    dst[tid.x] = BFloat16(float(tid.x));
}

// Capabilities print in the order the emitter inserts them, and the 8-bit
// request is considered before the 16-bit one, so a misclassification of
// BFloat16 as 8-bit would print an 8-bit line anywhere in the module. The
// leading NOT scans from module start to the first DAG match; the trailing
// NOT scans from the last match to the end.
// CHECK-NOT: OpCapability UniformAndStorageBuffer8BitAccess
// CHECK-DAG: OpCapability UniformAndStorageBuffer16BitAccess
// CHECK-DAG: OpCapability BFloat16TypeKHR
// CHECK-NOT: OpCapability UniformAndStorageBuffer8BitAccess
