//TEST:SIMPLE(filecheck=CHECK,exit-code=2): -target hlsl
//TEST:SIMPLE(filecheck=CHECK,exit-code=2): -target metal
//TEST:SIMPLE(filecheck=CHECK,exit-code=2): -target wgsl

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
// CHECK: expected an expression of type 'StructuredBuffer<uint, DefaultDataLayout>', got '__UnsizedStructuredBuffer<uint, DefaultDataLayout>'
// CHECK: type mismatch
// CHECK: expected an expression of type 'RWStructuredBuffer<uint, DefaultDataLayout>', got '__UnsizedRWStructuredBuffer<uint, DefaultDataLayout>'
// CHECK: type mismatch
// CHECK: expected an expression of type 'ByteAddressBuffer', got '__UnsizedByteAddressBuffer'
// CHECK: type mismatch
// CHECK: expected an expression of type 'RWByteAddressBuffer', got '__UnsizedRWByteAddressBuffer'
// CHECK: type mismatch
// CHECK: expected an expression of type '__UnsizedRWStructuredBuffer<uint, DefaultDataLayout>', got 'StructuredBuffer<uint, DefaultDataLayout>'
// CHECK: type mismatch
// CHECK: expected an expression of type '__UnsizedRWByteAddressBuffer', got 'ByteAddressBuffer'
