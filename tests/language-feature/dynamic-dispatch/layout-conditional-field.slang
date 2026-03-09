// Conditional<T, bool> as a field in a generic interface implementation,
// with two specializations dispatched dynamically. Conditional lowers to
// vector<T, 0 or 1>, producing different-sized AnyValue layouts per
// specialization.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgsl -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IVertex
{
    float3 getPosition();
    float getNormalLength();
    float getColorSum();
}

struct Vertex<let hasNormal : bool, let hasColor : bool> : IVertex
{
    float3 position;
    Conditional<float3, hasNormal> normal;
    Conditional<float3, hasColor> color;

    float3 getPosition() { return position; }

    float getNormalLength()
    {
        if (let n = normal.get())
            return length(n);
        return -1.0;
    }

    float getColorSum()
    {
        if (let c = color.get())
            return c.x + c.y + c.z;
        return -1.0;
    }
}

IVertex makeVertex(int id)
{
    if (id == 0)
    {
        Vertex<true, false> v;
        v.position = float3(1.0, 2.0, 3.0);
        v.normal = float3(0.0, 1.0, 0.0);
        return v;
    }
    else
    {
        Vertex<false, true> v;
        v.position = float3(4.0, 5.0, 6.0);
        v.color = float3(0.2, 0.4, 0.6);
        return v;
    }
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // Vertex<true, false>: has normal (0,1,0), no color
    var a = makeVertex(id);
    outputBuffer[0] = a.getPosition().x;
    // CHECK: 1.0
    outputBuffer[1] = a.getPosition().y;
    // CHECK: 2.0
    outputBuffer[2] = a.getNormalLength();
    // CHECK: 1.0
    outputBuffer[3] = a.getColorSum();
    // CHECK: -1.0

    // Vertex<false, true>: no normal, has color (0.2, 0.4, 0.6)
    var b = makeVertex(id + 1);
    outputBuffer[4] = b.getPosition().x;
    // CHECK: 4.0
    outputBuffer[5] = b.getPosition().y;
    // CHECK: 5.0
    outputBuffer[6] = b.getNormalLength();
    // CHECK: -1.0
    outputBuffer[7] = b.getColorSum();
    // CHECK: 1.2
}
