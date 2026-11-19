//TEST:SIMPLE(filecheck=CHECK,exit-code=2): -target hlsl

// Conversion cannot reconstruct a missing count or grant write access to a read-only resource.
void invalidConversions(
    __UnsizedStructuredBuffer<uint> values,
    __UnsizedRWStructuredBuffer<uint> writableValues,
    __UnsizedByteAddressBuffer bytes,
    __UnsizedRWByteAddressBuffer writableBytes,
    StructuredBuffer<uint> sizedValues,
    ByteAddressBuffer sizedBytes)
{
    StructuredBuffer<uint> sizedValuesAgain = values;
    RWStructuredBuffer<uint> sizedWritableValues = writableValues;
    ByteAddressBuffer sizedBytesAgain = bytes;
    RWByteAddressBuffer sizedWritableBytes = writableBytes;
    __UnsizedRWStructuredBuffer<uint> writableFromReadOnly = sizedValues;
    __UnsizedRWByteAddressBuffer writableBytesFromReadOnly = sizedBytes;
}

// CHECK: type mismatch
// CHECK: type mismatch
// CHECK: type mismatch
// CHECK: type mismatch
// CHECK: type mismatch
// CHECK: type mismatch
