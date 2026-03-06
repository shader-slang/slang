// Fixed-size array fields in interface implementations must be packed
// element-by-element into AnyValue storage. This tests that the marshalling
// correctly handles arrays of scalars with different element counts and types.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgsl -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IData
{
    float sum();
    float element(int i);
}

struct FloatArray : IData
{
    float data[4];

    float sum()
    {
        float s = 0.0;
        for (int i = 0; i < 4; i++)
            s += data[i];
        return s;
    }

    float element(int i) { return data[i]; }
}

struct IntPair : IData
{
    int ids[2];
    float scale;

    float sum()
    {
        return float(ids[0] + ids[1]) * scale;
    }

    float element(int i) { return float(ids[i]) * scale; }
}

IData makeData(int id)
{
    if (id == 0)
    {
        FloatArray a;
        a.data[0] = 1.0;
        a.data[1] = 2.0;
        a.data[2] = 3.0;
        a.data[3] = 4.0;
        return a;
    }
    else
    {
        IntPair p;
        p.ids[0] = 10;
        p.ids[1] = 20;
        p.scale = 0.5;
        return p;
    }
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    var a = makeData(id);
    outputBuffer[0] = a.sum();
    // CHECK: 10.0
    outputBuffer[1] = a.element(0);
    // CHECK: 1.0
    outputBuffer[2] = a.element(2);
    // CHECK: 3.0

    var b = makeData(id + 1);
    outputBuffer[3] = b.sum();
    // CHECK: 15.0
    outputBuffer[4] = b.element(0);
    // CHECK: 5.0
    outputBuffer[5] = b.element(1);
    // CHECK: 10.0

    // Verify array field independence after assignment
    var c = makeData(id);
    var d = c;
    outputBuffer[6] = d.sum();
    // CHECK: 10.0
    outputBuffer[7] = d.element(3);
    // CHECK: 4.0
}
