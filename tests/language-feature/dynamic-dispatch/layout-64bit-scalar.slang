// 64-bit scalar types (double, int64_t, uint64_t) are marshalled into
// AnyValue as two consecutive uint32 fields with 8-byte alignment.
// This test verifies that the split-and-reconstruct preserves values
// correctly through dynamic dispatch.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -render-feature double,int64

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:ubuffer(data=[0 1 2], stride=4):name=kindBuffer
StructuredBuffer<int> kindBuffer;

interface IValue
{
    float asFloat();
    int lowBits();
}

struct DoubleImpl : IValue
{
    double val;

    float asFloat() { return float(val); }
    int lowBits() { return int(val); }
}

struct Int64Impl : IValue
{
    int64_t val;

    float asFloat() { return float(val); }
    int lowBits() { return int(val & 0xFFFF); }
}

struct Uint64Impl : IValue
{
    uint64_t val;

    float asFloat() { return float(val); }
    int lowBits() { return int(val & 0xFFFF); }
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
    u.val = 99;
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

    IValue u64 = makeValue(kindBuffer[2]);
    // CHECK: 99.0
    outputBuffer[3] = u64.asFloat();
    // CHECK: 99
    outputBuffer[4] = u64.lowBits();

    // Verify negative int64 low bits with masking
    // -42 & 0xFFFF = 65494
    // CHECK: 65494
    outputBuffer[5] = i64.lowBits();
}
