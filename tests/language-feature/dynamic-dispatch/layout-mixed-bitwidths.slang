// Mixed bit-width fields exercise all alignment transitions in the
// AnyValue marshalling code: 1-byte (int8_t), 2-byte (half),
// 4-byte (float), and 8-byte (double) boundaries packed together
// in a single interface implementation.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -render-feature int16,half,double

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:ubuffer(data=[0 1], stride=4):name=kindBuffer
StructuredBuffer<int> kindBuffer;

interface IMixed
{
    float getSum();
    float getDoubleVal();
    float getByteFields();
}

struct MixedImpl : IMixed
{
    int8_t a;
    half b;
    float c;
    double d;
    uint8_t e;

    float getSum()
    {
        return float(a) + float(b) + c + float(d) + float(e);
    }
    float getDoubleVal() { return float(d); }
    float getByteFields() { return float(a) * 1000.0 + float(e); }
}

struct SimpleImpl : IMixed
{
    float val;

    float getSum() { return val; }
    float getDoubleVal() { return val; }
    float getByteFields() { return val; }
}

IMixed makeMixed(int kind)
{
    if (kind == 0)
    {
        MixedImpl m;
        m.a = 1;
        m.b = half(2.0);
        m.c = 3.0;
        m.d = 4.0;
        m.e = 5;
        return m;
    }
    SimpleImpl s;
    s.val = 77.0;
    return s;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    IMixed mixed = makeMixed(kindBuffer[0]);
    // 1 + 2 + 3 + 4 + 5 = 15
    // CHECK: 15.0
    outputBuffer[0] = mixed.getSum();
    // CHECK: 4.0
    outputBuffer[1] = mixed.getDoubleVal();
    // a=1, e=5: 1*1000 + 5 = 1005 (pins down byte-field placement)
    // CHECK: 1005.0
    outputBuffer[2] = mixed.getByteFields();

    IMixed simple = makeMixed(kindBuffer[1]);
    // CHECK: 77.0
    outputBuffer[3] = simple.getSum();
    // CHECK: 77.0
    outputBuffer[4] = simple.getDoubleVal();
}
