//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

StructuredBuffer<uint> values;
RWStructuredBuffer<uint> writableValues;
ByteAddressBuffer bytes;
RWByteAddressBuffer writableBytes;

[numthreads(1, 1, 1)]
void computeMain()
{
    let unusedValues = __UnsizedStructuredBuffer<uint>(values);
    let unusedWritableValues = __UnsizedRWStructuredBuffer<uint>(writableValues);
    let unusedBytes = __UnsizedByteAddressBuffer(bytes);
    let unusedWritableBytes = __UnsizedRWByteAddressBuffer(writableBytes);
}

// Conversion is pure: an unused result must not leave resource reads in the entry point.
// CHECK: void computeMain
// CHECK: {
// CHECK-NOT: values
// CHECK-NOT: writableValues
// CHECK-NOT: bytes
// CHECK-NOT: writableBytes
// CHECK: }
