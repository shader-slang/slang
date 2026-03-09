// Conditional<T, bool> as a field in a generic interface implementation,
// with four specializations of different payload sizes dispatched
// dynamically. Conditional<T, cond> lowers to a zero-size or T-sized
// field depending on the bool, so each specialization produces a
// different-sized AnyValue layout:
//   Vertex<true, true>   -> position + normal + color  (largest)
//   Vertex<true, false>  -> position + normal          (medium)
//   Vertex<false, true>  -> position + color           (medium)
//   Vertex<false, false> -> position only              (smallest)

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgsl -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
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
    switch (id)
    {
    case 0:
    {
        Vertex<true, true> v;
        v.position = float3(1.0, 2.0, 3.0);
        v.normal = float3(0.0, 1.0, 0.0);
        v.color = float3(0.2, 0.4, 0.6);
        return v;
    }
    case 1:
    {
        Vertex<true, false> v;
        v.position = float3(4.0, 5.0, 6.0);
        v.normal = float3(0.0, 0.0, 1.0);
        return v;
    }
    case 2:
    {
        Vertex<false, true> v;
        v.position = float3(10.0, 11.0, 12.0);
        v.color = float3(0.1, 0.3, 0.5);
        return v;
    }
    default:
    {
        Vertex<false, false> v;
        v.position = float3(7.0, 8.0, 9.0);
        return v;
    }
    }
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // Vertex<true, true>: has normal (0,1,0) and color (0.2, 0.4, 0.6)
    var a = makeVertex(id);
    outputBuffer[0] = a.getPosition().x;
    // CHECK: 1.0
    outputBuffer[1] = a.getNormalLength();
    // CHECK: 1.0
    outputBuffer[2] = a.getColorSum();
    // CHECK: 1.2
    outputBuffer[3] = a.getPosition().z;
    // CHECK: 3.0

    // Vertex<true, false>: has normal (0,0,1), no color
    var b = makeVertex(id + 1);
    outputBuffer[4] = b.getPosition().x;
    // CHECK: 4.0
    outputBuffer[5] = b.getNormalLength();
    // CHECK: 1.0
    outputBuffer[6] = b.getColorSum();
    // CHECK: -1.0
    outputBuffer[7] = b.getPosition().z;
    // CHECK: 6.0

    // Vertex<false, true>: no normal, has color (0.1, 0.3, 0.5)
    var c = makeVertex(id + 2);
    outputBuffer[8] = c.getPosition().x;
    // CHECK: 10.0
    outputBuffer[9] = c.getNormalLength();
    // CHECK: -1.0
    outputBuffer[10] = c.getColorSum();
    // CHECK: 0.9
    outputBuffer[11] = c.getPosition().z;
    // CHECK: 12.0

    // Vertex<false, false>: no normal, no color (smallest payload)
    var d = makeVertex(id + 3);
    outputBuffer[12] = d.getPosition().x;
    // CHECK: 7.0
    outputBuffer[13] = d.getNormalLength();
    // CHECK: -1.0
    outputBuffer[14] = d.getColorSum();
    // CHECK: -1.0
    outputBuffer[15] = d.getPosition().z;
    // CHECK: 9.0
}
