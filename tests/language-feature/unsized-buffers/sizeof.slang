//TEST(64-bit):SIMPLE(filecheck=COMPILE): -target cuda -entry computeMain -stage compute
//TEST(64-bit):SIMPLE(filecheck=COMPILE): -target cpp -entry computeMain -stage compute
//TEST(compute,64-bit):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -compute -output-using-type
//TEST(compute,64-bit):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cuda -compute -output-using-type
//TEST(compute,64-bit):COMPARE_COMPUTE(filecheck-buffer=CHECK): -llvm -compute -output-using-type

struct PointerBuffers
{
    __UnsizedStructuredBuffer<uint> values;
    __UnsizedRWStructuredBuffer<uint> writableValues;
    __UnsizedByteAddressBuffer bytes;
    __UnsizedRWByteAddressBuffer writableBytes;
    uint marker;
}

struct StructuredBufferArray
{
    __UnsizedStructuredBuffer<uint> values[2];
}

struct ByteAddressBufferArray
{
    __UnsizedByteAddressBuffer values[2];
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=output
RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = __sizeOf<__UnsizedStructuredBuffer<uint>>();
    output[1] = __sizeOf<__UnsizedRWStructuredBuffer<uint>>();
    output[2] = __sizeOf<__UnsizedByteAddressBuffer>();
    output[3] = __sizeOf<__UnsizedRWByteAddressBuffer>();
    output[4] = __sizeOf<PointerBuffers>();
    output[5] = __alignOf<PointerBuffers>();
    output[6] = __sizeOf<StructuredBufferArray>();
    output[7] = __sizeOf<ByteAddressBufferArray>();
    output[8] = __sizeOf<StructuredBuffer<uint>>();
    output[9] = __sizeOf<RWStructuredBuffer<uint>>();
    output[10] = __sizeOf<ByteAddressBuffer>();
    output[11] = __sizeOf<RWByteAddressBuffer>();
}

// COMPILE: computeMain
// CHECK: 8
// CHECK: 8
// CHECK: 8
// CHECK: 8
// CHECK: 40
// CHECK: 8
// CHECK: 16
// CHECK: 16
// CHECK: 16
// CHECK: 16
// CHECK: 16
// CHECK: 16
