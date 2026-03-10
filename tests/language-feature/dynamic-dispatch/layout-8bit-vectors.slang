// Vectors of 8-bit types are packed via bitfield insert into uint32 fields.
// A vector<int8_t, 4> fills exactly one uint32; a vector<int8_t, 3> uses
// 3 bytes leaving 1 byte unused. This tests the intraFieldOffset
// arithmetic in AnyValue marshalling for sub-byte-aligned elements.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -render-feature int16
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT:ubuffer(data=[0 1 2], stride=4):name=kindBuffer
StructuredBuffer<int> kindBuffer;

interface I8BitVec
{
    int getSum();
    int getFirst();
}

struct Vec4i8Impl : I8BitVec
{
    vector<int8_t, 4> vals;

    int getSum()
    {
        return int(vals[0]) + int(vals[1]) + int(vals[2]) + int(vals[3]);
    }
    int getFirst() { return int(vals[0]); }
}

struct Vec2u8Impl : I8BitVec
{
    vector<uint8_t, 2> vals;

    int getSum()
    {
        return int(vals[0]) + int(vals[1]);
    }
    int getFirst() { return int(vals[0]); }
}

struct Vec3i8Impl : I8BitVec
{
    vector<int8_t, 3> vals;

    int getSum()
    {
        return int(vals[0]) + int(vals[1]) + int(vals[2]);
    }
    int getFirst() { return int(vals[0]); }
}

I8BitVec make8BitVec(int kind)
{
    if (kind == 0)
    {
        Vec4i8Impl v;
        v.vals = vector<int8_t, 4>(int8_t(10), int8_t(20), int8_t(30), int8_t(40));
        return v;
    }
    if (kind == 1)
    {
        Vec2u8Impl v;
        v.vals = vector<uint8_t, 2>(uint8_t(100), uint8_t(200));
        return v;
    }
    Vec3i8Impl v;
    v.vals = vector<int8_t, 3>(int8_t(-1), int8_t(-2), int8_t(-3));
    return v;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    I8BitVec v4 = make8BitVec(kindBuffer[0]);
    // 10 + 20 + 30 + 40 = 100
    // CHECK: 100
    outputBuffer[0] = v4.getSum();
    // CHECK: 10
    outputBuffer[1] = v4.getFirst();

    I8BitVec v2 = make8BitVec(kindBuffer[1]);
    // 100 + 200 = 300
    // CHECK: 300
    outputBuffer[2] = v2.getSum();

    I8BitVec v3 = make8BitVec(kindBuffer[2]);
    // (-1) + (-2) + (-3) = -6
    // CHECK: -6
    outputBuffer[3] = v3.getSum();
    // CHECK: -1
    outputBuffer[4] = v3.getFirst();

    // Re-read vec4 to verify no corruption from smaller vector impls
    I8BitVec v4b = make8BitVec(kindBuffer[0]);
    // CHECK: 100
    outputBuffer[5] = v4b.getSum();
}
