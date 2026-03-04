//TEST:SIMPLE(filecheck=CHECK):-target spirv -emit-spirv-directly

// When using a write-only texture (WTexture2D) without an explicit format,
// only StorageImageWriteWithoutFormat should be required, not
// StorageImageReadWithoutFormat.
// See: https://github.com/shader-slang/slang/issues/9997

// CHECK-NOT: OpCapability StorageImageReadWithoutFormat
// CHECK: OpCapability StorageImageWriteWithoutFormat
// CHECK: OpDecorate {{.*}} NonReadable
// CHECK-NOT: OpCapability StorageImageReadWithoutFormat

WTexture2D writeOnlyTexture;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    writeOnlyTexture.Store(int2(0, 0), float4(0, 0, 0, 1));
}
