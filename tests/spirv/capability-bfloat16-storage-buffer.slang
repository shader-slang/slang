// BFloat16 is a 16-bit floating-point element type, so a storage buffer
// holding it must declare the 16-bit storage capability.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute

RWStructuredBuffer<BFloat16> dst;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    dst[tid.x] = BFloat16(float(tid.x));
}

// CHECK-NOT: OpCapability UniformAndStorageBuffer8BitAccess
// CHECK-DAG: OpCapability UniformAndStorageBuffer16BitAccess
// CHECK-DAG: OpCapability BFloat16TypeKHR
// CHECK-NOT: OpCapability UniformAndStorageBuffer8BitAccess
