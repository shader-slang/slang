//TEST:SIMPLE(filecheck=CHECK):-target spirv -emit-spirv-directly

// When using a read-write texture (RWTexture2D) without an explicit format,
// both StorageImageReadWithoutFormat and StorageImageWriteWithoutFormat
// should be required.
// Companion to write-only-image-capability.slang.

// CHECK-DAG: OpCapability StorageImageReadWithoutFormat
// CHECK-DAG: OpCapability StorageImageWriteWithoutFormat

RWTexture2D<float4> rwTexture;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    rwTexture[int2(0, 0)] = rwTexture[int2(1, 1)];
}
