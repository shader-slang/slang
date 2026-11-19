//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain -validate-ir -O0
//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain -validate-ir -O0 -DDIRECT=1
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain -validate-ir
//TEST(compute,metal):COMPARE_COMPUTE(filecheck-buffer=RESULT): -metal -compute -output-using-type
//TEST(compute,metal):COMPARE_COMPUTE(filecheck-buffer=RESULT): -metal -compute -output-using-type -xslang -DDIRECT=1

// Compile both direct unsized resources and sized resources converted at call boundaries.
// Metal and WGSL retain their native resource representation in both cases.
#ifdef DIRECT
#define READ_VALUES __UnsizedStructuredBuffer<uint>
#define WRITE_VALUES __UnsizedRWStructuredBuffer<uint>
#define READ_BYTES __UnsizedByteAddressBuffer
#define WRITE_BYTES __UnsizedRWByteAddressBuffer
#else
#define READ_VALUES StructuredBuffer<uint>
#define WRITE_VALUES RWStructuredBuffer<uint>
#define READ_BYTES ByteAddressBuffer
#define WRITE_BYTES RWByteAddressBuffer
#endif

//TEST_INPUT:ubuffer(data=[10 20], stride=4):name=readValues
[[vk::binding(0, 0)]] READ_VALUES readValues;
//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=writeValues
[[vk::binding(1, 0)]] WRITE_VALUES writeValues;
//TEST_INPUT:ubuffer(data=[0 100 200], stride=4):name=readBytes
[[vk::binding(2, 0)]] READ_BYTES readBytes;
//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):name=writeBytes
[[vk::binding(3, 0)]] WRITE_BYTES writeBytes;

[noinline]
void copyValues(
    __UnsizedStructuredBuffer<uint> source,
    __UnsizedRWStructuredBuffer<uint> destination,
    __UnsizedByteAddressBuffer byteSource,
    __UnsizedRWByteAddressBuffer byteDestination,
    uint index)
{
    uint offset = index * 4 + 4;
    uint value = source.Load(index) + byteSource.Load(offset);
    byteDestination.Store(offset, value + 1);
    destination[index] = byteDestination.Load(offset);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // In the sized-resource mode, the first call converts all four resources implicitly.
    copyValues(readValues, writeValues, readBytes, writeBytes, 0);
#ifdef DIRECT
    copyValues(readValues, writeValues, readBytes, writeBytes, 1);
#else
    copyValues(
        __UnsizedStructuredBuffer<uint>(readValues),
        __UnsizedRWStructuredBuffer<uint>(writeValues),
        __UnsizedByteAddressBuffer(readBytes),
        __UnsizedRWByteAddressBuffer(writeBytes),
        1);
#endif
}

// RESULT: 111
// RESULT: 221

// The helper receives only the four device pointers and the user index. No buffer count
// or CPU/CUDA prelude wrapper may appear in this native Metal calling convention.
// METAL: void copyValues{{[_0-9]*}}(const uint device* source{{[_0-9]*}}, uint device* destination{{[_0-9]*}}, const uint32_t device* byteSource{{[_0-9]*}}, uint32_t device* byteDestination{{[_0-9]*}}, uint index{{[_0-9]*}})
// METAL: byteDestination{{[_0-9]*}}[({{.*}})>>2] =
// METAL-SAME: source{{[_0-9]*}}[index{{[_0-9]*}}]
// METAL-SAME: byteSource{{[_0-9]*}}[({{.*}})>>2]
// METAL: *(destination{{[_0-9]*}}+index{{[_0-9]*}}) =
// METAL-SAME: byteDestination{{[_0-9]*}}[({{.*}})>>2]
// METAL: void computeMain(const uint device* readValues{{[_0-9]*}} {{\[\[}}buffer(0){{\]\]}}, uint device* writeValues{{[_0-9]*}} {{\[\[}}buffer(1){{\]\]}}, const uint32_t device* readBytes{{[_0-9]*}} {{\[\[}}buffer(2){{\]\]}}, uint32_t device* writeBytes{{[_0-9]*}} {{\[\[}}buffer(3){{\]\]}})
// METALLIB: define void @computeMain
