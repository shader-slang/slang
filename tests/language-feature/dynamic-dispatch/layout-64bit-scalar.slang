// 64-bit scalar types (double, int64_t, uint64_t) are marshalled into
// AnyValue as two consecutive uint32 fields with 8-byte alignment.
// This test verifies that the split-and-reconstruct preserves values
// correctly through dynamic dispatch.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -render-feature double,int64

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:ubuffer(data=[0 1 2], stride=4):name=kindBuffer
StructuredBuffer<int> kindBuffer;

interface IValue
{
    float asFloat();
    int lowBits();
    int highBits();
}

struct DoubleImpl : IValue
{
    double val;

    float asFloat() { return float(val); }
    int lowBits() { return int(int64_t(val) & 0xFFFF); }
    int highBits() { return int(int64_t(val) >> 32); }
}

struct Int64Impl : IValue
{
    int64_t val;

    float asFloat() { return float(val); }
    int lowBits() { return int(val & 0xFFFF); }
    int highBits() { return int(val >> 32); }
}

struct Uint64Impl : IValue
{
    uint64_t val;

    float asFloat() { return float(val); }
    int lowBits() { return int(val & 0xFFFF); }
    int highBits() { return int(val >> 32); }
}

IValue makeValue(int kind)
{
    if (kind == 0)
    {
        DoubleImpl d;
        d.val = 3.25;
        return d;
    }
    if (kind == 1)
    {
        Int64Impl i;
        i.val = -42;
        return i;
    }
    Uint64Impl u;
    u.val = (uint64_t(5) << 32) | uint64_t(99);
    return u;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    IValue dbl = makeValue(kindBuffer[0]);
    // CHECK: 3.25
    outputBuffer[0] = dbl.asFloat();
    // CHECK: 3
    outputBuffer[1] = dbl.lowBits();

    IValue i64 = makeValue(kindBuffer[1]);
    // CHECK: -42.0
    outputBuffer[2] = i64.asFloat();
    // -42 & 0xFFFF = 65494
    // CHECK: 65494
    outputBuffer[3] = i64.lowBits();
    // -42 >> 32 = -1 (sign extension)
    // CHECK: -1
    outputBuffer[4] = i64.highBits();

    IValue u64 = makeValue(kindBuffer[2]);
    // (5 << 32) | 99: lowBits = 99
    // CHECK: 99
    outputBuffer[5] = u64.lowBits();
    // highBits = 5 (non-zero upper word proves no 32-bit truncation)
    // CHECK: 5
    outputBuffer[6] = u64.highBits();
}
