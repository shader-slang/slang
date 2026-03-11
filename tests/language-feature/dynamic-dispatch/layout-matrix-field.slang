// Matrix fields in interface implementations exercise composite-of-composite
// AnyValue packing. Matrices are stored internally as arrays of column vectors,
// so each matrix field requires nested marshalling of its vector elements.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgpu -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IMatField
{
    float readRC(int r, int c);
    float computeTrace();
    float transformAndSum();
}

struct Rotation3x3 : IMatField
{
    float3x3 rotation;

    float readRC(int r, int c) { return rotation[r][c]; }

    float computeTrace()
    {
        return rotation[0][0] + rotation[1][1] + rotation[2][2];
    }

    float transformAndSum()
    {
        float3 v = float3(1.0, 1.0, 1.0);
        float3 r = mul(rotation, v);
        return r.x + r.y + r.z;
    }
}

struct Transform4x4 : IMatField
{
    float4x4 transform;

    float readRC(int r, int c) { return transform[r][c]; }

    float computeTrace()
    {
        return transform[0][0] + transform[1][1] + transform[2][2] + transform[3][3];
    }

    float transformAndSum()
    {
        float4 v = float4(1.0, 1.0, 1.0, 1.0);
        float4 r = mul(transform, v);
        return r.x + r.y + r.z + r.w;
    }
}

IMatField makeMat(int id)
{
    if (id == 0)
    {
        Rotation3x3 a;
        a.rotation = float3x3(
            2.0, 0.0, 0.0,
            0.0, 3.0, 0.0,
            0.0, 0.0, 4.0);
        return a;
    }
    else
    {
        Transform4x4 b;
        b.transform = float4x4(
            1.0, 0.0, 0.0, 5.0,
            0.0, 1.0, 0.0, 6.0,
            0.0, 0.0, 1.0, 7.0,
            0.0, 0.0, 0.0, 1.0);
        return b;
    }
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // Rotation3x3: diag(2, 3, 4)
    var a = makeMat(id);
    outputBuffer[0] = a.readRC(0, 0);
    // CHECK: 2.0
    outputBuffer[1] = a.readRC(1, 1);
    // CHECK: 3.0
    outputBuffer[2] = a.readRC(0, 1);
    // CHECK: 0.0
    outputBuffer[3] = a.computeTrace();
    // CHECK: 9.0
    outputBuffer[4] = a.transformAndSum();
    // CHECK: 9.0

    // Transform4x4: translation(5, 6, 7)
    var b = makeMat(id + 1);
    outputBuffer[5] = b.readRC(0, 0);
    // CHECK: 1.0
    outputBuffer[6] = b.readRC(0, 3);
    // CHECK: 5.0
    outputBuffer[7] = b.readRC(3, 3);
    // CHECK: 1.0
    outputBuffer[8] = b.computeTrace();
    // CHECK: 4.0
    outputBuffer[9] = b.transformAndSum();
    // CHECK: 22.0
}
