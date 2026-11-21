//TEST(64-bit):REFLECTION(filecheck=CHECK): -target cuda -entry computeMain -stage compute -no-codegen
//TEST(64-bit):REFLECTION(filecheck=CHECK): -target host-callable -entry computeMain -stage compute -no-codegen

// Mixed fields and arrays expose both the size and alignment of each buffer representation.
// In particular, an unsized buffer array has a pointer stride, while sized fields retain their ABI.
struct BufferParams
{
    __UnsizedStructuredBuffer<uint> readValues;
    __UnsizedRWStructuredBuffer<uint> writeValues;
    __UnsizedByteAddressBuffer readBytes;
    __UnsizedRWByteAddressBuffer writeBytes;
    StructuredBuffer<uint> sizedValues;
    RWStructuredBuffer<uint> sizedRWValues;
    ByteAddressBuffer sizedBytes;
    RWByteAddressBuffer sizedRWBytes;
    __UnsizedStructuredBuffer<uint> valueArray[2];
    __UnsizedRWStructuredBuffer<uint> rwValueArray[2];
    __UnsizedByteAddressBuffer byteArray[2];
    __UnsizedRWByteAddressBuffer rwByteArray[2];
    uint sentinel;
}

[numthreads(1, 1, 1)]
void computeMain(uniform BufferParams params)
{
}

// CHECK: "name": "params",
// CHECK: "binding": {"kind": "uniform", "offset": 0, "size": 168
// CHECK: "name": "readValues",
// CHECK: "binding": {"kind": "uniform", "offset": 0, "size": 8
// CHECK: "name": "writeValues",
// CHECK: "binding": {"kind": "uniform", "offset": 8, "size": 8
// CHECK: "name": "readBytes",
// CHECK: "binding": {"kind": "uniform", "offset": 16, "size": 8
// CHECK: "name": "writeBytes",
// CHECK: "binding": {"kind": "uniform", "offset": 24, "size": 8
// CHECK: "name": "sizedValues",
// CHECK: "binding": {"kind": "uniform", "offset": 32, "size": 16
// CHECK: "name": "sizedRWValues",
// CHECK: "binding": {"kind": "uniform", "offset": 48, "size": 16
// CHECK: "name": "sizedBytes",
// CHECK: "binding": {"kind": "uniform", "offset": 64, "size": 16
// CHECK: "name": "sizedRWBytes",
// CHECK: "binding": {"kind": "uniform", "offset": 80, "size": 16
// CHECK: "name": "valueArray",
// CHECK: "binding": {"kind": "uniform", "offset": 96, "size": 16, "elementStride": 8
// CHECK: "name": "rwValueArray",
// CHECK: "binding": {"kind": "uniform", "offset": 112, "size": 16, "elementStride": 8
// CHECK: "name": "byteArray",
// CHECK: "binding": {"kind": "uniform", "offset": 128, "size": 16, "elementStride": 8
// CHECK: "name": "rwByteArray",
// CHECK: "binding": {"kind": "uniform", "offset": 144, "size": 16, "elementStride": 8
// CHECK: "name": "sentinel",
// CHECK: "binding": {"kind": "uniform", "offset": 160, "size": 4
