// Conditional<T, false> produces a zero-size field. This verifies that
// AnyValue packing handles a zero-size Conditional alongside other
// concrete fields correctly.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgsl -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IRecord
{
    float getX();
    float getY();
    float getExtra();
}

struct FullRecord : IRecord
{
    float x;
    Conditional<float3, true> extra;
    float y;

    float getX() { return x; }
    float getY() { return y; }

    float getExtra()
    {
        if (let e = extra.get())
            return e.x + e.y + e.z;
        return -1.0;
    }
}

struct SlimRecord : IRecord
{
    float x;
    Conditional<float3, false> noExtra;
    float y;

    float getX() { return x; }
    float getY() { return y; }

    float getExtra()
    {
        if (let e = noExtra.get())
            return e.x + e.y + e.z;
        return -1.0;
    }
}

IRecord makeRecord(int id)
{
    if (id == 0)
    {
        FullRecord r;
        r.x = 10.0;
        r.extra = float3(1.0, 2.0, 3.0);
        r.y = 20.0;
        return r;
    }
    else
    {
        SlimRecord r;
        r.x = 100.0;
        r.y = 200.0;
        return r;
    }
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // FullRecord: x=10, extra=(1,2,3), y=20
    var a = makeRecord(id);
    outputBuffer[0] = a.getX();
    // CHECK: 10.0
    outputBuffer[1] = a.getY();
    // CHECK: 20.0
    outputBuffer[2] = a.getExtra();
    // CHECK: 6.0

    // SlimRecord: x=100, noExtra (zero-size), y=200
    var b = makeRecord(id + 1);
    outputBuffer[3] = b.getX();
    // CHECK: 100.0
    outputBuffer[4] = b.getY();
    // CHECK: 200.0
    outputBuffer[5] = b.getExtra();
    // CHECK: -1.0
}
