//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk

interface IVertex
{
    property float3 position{get;}
    property Optional<float3> normal{get;}
    property Optional<float3> color{get;}
}

struct Vertex<bool hasNormal, bool hasColor> : IVertex
{
    private float3 m_position;
    private float3 m_normal[hasNormal];
    private float3 m_color[hasColor];

    __init(float3 position, float3 normal, float3 color)
    {
        m_position = position;
        if (hasNormal) m_normal[0] = normal;
        if (hasColor) m_color[0] = color;
    }

    property float3 position
    {
        get { return m_position; }
    }
    property Optional<float3> normal
    {
        get { if (hasNormal) return m_normal[0]; else return none; }
    }
    property Optional<float3> color
    {
        get { if (hasColor) return m_color[0]; else return none; }
    }
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

void test<V:IVertex>(V vert)
{
    // CHECK: 0
    // CHECK: 0
    // CHECK: 1
    // CHECK: 3
    if (let normal = vert.normal)
    {
         outputBuffer[0] = 1;
         outputBuffer[1] = (int)normal.x;
    }

    if (let color = vert.color)
    {
        outputBuffer[2] = 1;
        outputBuffer[3] = (int)color.x;
    }
}

[numthreads(1,1,1)]
void computeMain()
{
    test<Vertex<false, true>>(Vertex<false, true>(1.0, 2.0, 3.0));
}