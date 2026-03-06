// Arrays of vector types as impl fields combine array iteration with
// per-element vector packing. Each vector element occupies aligned storage,
// and the array layout must preserve this alignment for every element.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IMesh
{
    float3 getNormal(int i);
    float averageY();
}

struct QuadMesh : IMesh
{
    float3 normals[4];

    float3 getNormal(int i) { return normals[i]; }

    float averageY()
    {
        float s = 0.0;
        for (int i = 0; i < 4; i++)
            s += normals[i].y;
        return s * 0.25;
    }
}

struct LineMesh : IMesh
{
    float3 normals[2];

    float3 getNormal(int i) { return normals[i]; }

    float averageY()
    {
        return (normals[0].y + normals[1].y) * 0.5;
    }
}

IMesh makeMesh(int id)
{
    if (id == 0)
    {
        QuadMesh m;
        m.normals[0] = float3(1.0, 0.0, 0.0);
        m.normals[1] = float3(0.0, 1.0, 0.0);
        m.normals[2] = float3(0.0, 0.0, 1.0);
        m.normals[3] = float3(1.0, 1.0, 0.0);
        return m;
    }
    else
    {
        LineMesh m;
        m.normals[0] = float3(0.0, 3.0, 0.0);
        m.normals[1] = float3(0.0, 5.0, 0.0);
        return m;
    }
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // QuadMesh normals: (1,0,0), (0,1,0), (0,0,1), (1,1,0)
    var q = makeMesh(id);
    float3 n0 = q.getNormal(0);
    outputBuffer[0] = n0.x;
    // CHECK: 1.0
    outputBuffer[1] = n0.y;
    // CHECK: 0.0

    float3 n2 = q.getNormal(2);
    outputBuffer[2] = n2.z;
    // CHECK: 1.0

    outputBuffer[3] = q.averageY();
    // CHECK: 0.5

    // LineMesh normals: (0,3,0), (0,5,0)
    var l = makeMesh(id + 1);
    float3 ln0 = l.getNormal(0);
    outputBuffer[4] = ln0.y;
    // CHECK: 3.0

    float3 ln1 = l.getNormal(1);
    outputBuffer[5] = ln1.y;
    // CHECK: 5.0

    outputBuffer[6] = l.averageY();
    // CHECK: 4.0
}
