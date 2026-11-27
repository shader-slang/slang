//TEST:SIMPLE(filecheck=CHECK,exit-code=2): -target hlsl
//TEST:SIMPLE(filecheck=CHECK,exit-code=2): -target metal
//TEST:SIMPLE(filecheck=CHECK,exit-code=2): -target wgsl
//TEST:SIMPLE(filecheck=CHECK,exit-code=2): -target cuda

void invalidQueries(
    __UnsizedStructuredBuffer<uint> values,
    __UnsizedRWStructuredBuffer<uint> writableValues,
    __UnsizedByteAddressBuffer bytes,
    __UnsizedRWByteAddressBuffer writableBytes)
{
    uint count, stride;
    values.GetDimensions(count, stride);
    writableValues.GetDimensions(count, stride);
    bytes.GetDimensions(count);
    writableBytes.GetDimensions(count);
    values.getCount();
    writableValues.getCount();
}

// CHECK: 'GetDimensions' is not a member of
// CHECK: 'GetDimensions' is not a member of
// CHECK: 'GetDimensions' is not a member of
// CHECK: 'GetDimensions' is not a member of
// CHECK: 'getCount' is not a member of
// CHECK: 'getCount' is not a member of
