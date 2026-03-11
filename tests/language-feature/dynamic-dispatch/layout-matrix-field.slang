// Matrix fields in interface implementations exercise composite-of-composite
// AnyValue packing. Matrices are stored internally as arrays of column vectors,
// so each matrix field requires nested marshalling of its vector elements.
// Covers float2x2, float3x3, and float4x4 with non-diagonal entries to
// catch row/column ordering bugs in the packed representation.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgpu -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IMatField
{
    float readRC(int r, int c);
    float computeTrace();
    float transformAndSum();
    float extraCheck();
}

struct Small2x2 : IMatField
{
    float2x2 mat;

    float readRC(int r, int c) { return mat[r][c]; }
    float computeTrace() { return mat[0][0] + mat[1][1]; }

    float transformAndSum()
    {
        float2 v = float2(1.0, 1.0);
        float2 r = mul(mat, v);
        return r.x + r.y;
    }

    float extraCheck()
    {
        return mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0];
    }
}

struct Upper3x3 : IMatField
{
    float3x3 rotation;

    float readRC(int r, int c) { return rotation[r][c]; }
    float computeTrace() { return rotation[0][0] + rotation[1][1] + rotation[2][2]; }

    float transformAndSum()
    {
        float3 v = float3(1.0, 1.0, 1.0);
        float3 r = mul(rotation, v);
        return r.x + r.y + r.z;
    }

    float extraCheck() { return rotation[0][1]; }
}

struct Shear4x4 : IMatField
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

    float extraCheck() { return transform[1][2]; }
}

IMatField makeMat(int id)
{
    switch (id)
    {
    case 0:
    {
        Small2x2 a;
        a.mat = float2x2(
            3.0, 1.0,
            2.0, 5.0);
        return a;
    }
    case 1:
    {
        Upper3x3 b;
        b.rotation = float3x3(
            1.0, 2.0, 0.0,
            0.0, 3.0, 0.0,
            0.0, 1.0, 5.0);
        return b;
    }
    default:
    {
        Shear4x4 c;
        c.transform = float4x4(
            1.0, 0.0, 0.0, 5.0,
            0.0, 1.0, 3.0, 6.0,
            0.0, 0.0, 1.0, 7.0,
            0.0, 0.0, 0.0, 1.0);
        return c;
    }
    }
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // Small2x2: [[3,1],[2,5]], det=13, trace=8
    var a = makeMat(id);
    outputBuffer[0] = a.readRC(0, 0);
    // CHECK: 3.0
    outputBuffer[1] = a.readRC(0, 1);
    // CHECK: 1.0
    outputBuffer[2] = a.readRC(1, 0);
    // CHECK: 2.0
    outputBuffer[3] = a.computeTrace();
    // CHECK: 8.0
    outputBuffer[4] = a.transformAndSum();
    // CHECK: 11.0
    outputBuffer[5] = a.extraCheck();
    // CHECK: 13.0

    // Upper3x3: [[1,2,0],[0,3,0],[0,1,5]], trace=9
    var b = makeMat(id + 1);
    outputBuffer[6] = b.readRC(0, 1);
    // CHECK: 2.0
    outputBuffer[7] = b.readRC(2, 1);
    // CHECK: 1.0
    outputBuffer[8] = b.computeTrace();
    // CHECK: 9.0
    outputBuffer[9] = b.transformAndSum();
    // CHECK: 12.0
    outputBuffer[10] = b.extraCheck();
    // CHECK: 2.0

    // Shear4x4: identity + shear(1,2)=3 + translation(5,6,7), trace=4
    var c = makeMat(id + 2);
    outputBuffer[11] = c.readRC(0, 3);
    // CHECK: 5.0
    outputBuffer[12] = c.readRC(1, 2);
    // CHECK: 3.0
    outputBuffer[13] = c.computeTrace();
    // CHECK: 4.0
    outputBuffer[14] = c.transformAndSum();
    // CHECK: 25.0
    outputBuffer[15] = c.extraCheck();
    // CHECK: 3.0
}
