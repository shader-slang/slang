// Vectors of 64-bit types are marshalled element-by-element, with each
// element requiring 8-byte alignment. A double2 occupies 4 uint32 fields
// in the AnyValue buffer.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -render-feature double,int64

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:ubuffer(data=[0 1], stride=4):name=kindBuffer
StructuredBuffer<int> kindBuffer;

interface IVec
{
    float getX();
    float getY();
}

struct Double2Impl : IVec
{
    double2 val;

    float getX() { return float(val.x); }
    float getY() { return float(val.y); }
}

struct Int64_2Impl : IVec
{
    vector<int64_t, 2> val;

    float getX() { return float(val.x); }
    float getY() { return float(val.y); }
}

IVec makeVec(int kind)
{
    if (kind == 0)
    {
        Double2Impl d;
        d.val = double2(1.5, 2.75);
        return d;
    }
    Int64_2Impl i;
    i.val = vector<int64_t, 2>(-10, 20);
    return i;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    IVec dblVec = makeVec(kindBuffer[0]);
    // CHECK: 1.5
    outputBuffer[0] = dblVec.getX();
    // CHECK: 2.75
    outputBuffer[1] = dblVec.getY();

    IVec i64Vec = makeVec(kindBuffer[1]);
    // CHECK: -10.0
    outputBuffer[2] = i64Vec.getX();
    // CHECK: 20.0
    outputBuffer[3] = i64Vec.getY();

    // Cross-check: re-read double impl to verify no stale data
    IVec dbl2 = makeVec(kindBuffer[0]);
    // CHECK: 1.5
    outputBuffer[4] = dbl2.getX();
    // CHECK: 2.75
    outputBuffer[5] = dbl2.getY();
}
