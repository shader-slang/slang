// Vectors of 16-bit types pack two elements per uint32 field in AnyValue.
// A half4 occupies 2 uint32 fields; a vector<uint16_t, 3> uses 2 uint32
// fields with the second half-utilized. This tests the 2-byte alignment
// logic with different element counts.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -render-feature int16,half
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -dx12 -profile sm_6_2 -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:ubuffer(data=[0 1 2], stride=4):name=kindBuffer
StructuredBuffer<int> kindBuffer;

interface I16BitVec
{
    float getSum();
    float getFirst();
}

struct Half4Impl : I16BitVec
{
    half4 vals;

    float getSum()
    {
        return float(vals[0]) + float(vals[1]) + float(vals[2]) + float(vals[3]);
    }
    float getFirst() { return float(vals[0]); }
}

struct Int16_2Impl : I16BitVec
{
    int16_t2 vals;

    float getSum()
    {
        return float(vals[0]) + float(vals[1]);
    }
    float getFirst() { return float(vals[0]); }
}

struct Uint16_3Impl : I16BitVec
{
    vector<uint16_t, 3> vals;

    float getSum()
    {
        return float(vals[0]) + float(vals[1]) + float(vals[2]);
    }
    float getFirst() { return float(vals[0]); }
}

I16BitVec make16BitVec(int kind)
{
    if (kind == 0)
    {
        Half4Impl h;
        h.vals = half4(half(1.5), half(2.5), half(3.5), half(4.5));
        return h;
    }
    if (kind == 1)
    {
        Int16_2Impl i;
        i.vals = int16_t2(int16_t(-10), int16_t(20));
        return i;
    }
    Uint16_3Impl u;
    u.vals = vector<uint16_t, 3>(uint16_t(100), uint16_t(200), uint16_t(300));
    return u;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    I16BitVec h4 = make16BitVec(kindBuffer[0]);
    // 1.5 + 2.5 + 3.5 + 4.5 = 12.0
    // CHECK: 12.0
    outputBuffer[0] = h4.getSum();
    // CHECK: 1.5
    outputBuffer[1] = h4.getFirst();

    I16BitVec i2 = make16BitVec(kindBuffer[1]);
    // -10 + 20 = 10
    // CHECK: 10.0
    outputBuffer[2] = i2.getSum();

    I16BitVec u3 = make16BitVec(kindBuffer[2]);
    // 100 + 200 + 300 = 600
    // CHECK: 600.0
    outputBuffer[3] = u3.getSum();
    // CHECK: 100.0
    outputBuffer[4] = u3.getFirst();

    // Re-read half4 to verify no corruption
    I16BitVec h4b = make16BitVec(kindBuffer[0]);
    // CHECK: 12.0
    outputBuffer[5] = h4b.getSum();
}
