// Mixed vector, matrix, and scalar fields in the same interface implementation.
// This verifies that the AnyValue layout engine correctly composes different-sized
// types within a single struct and preserves field ordering across pack/unpack.
// Both matrices use non-diagonal entries to catch row/column ordering bugs.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgpu -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IMixed
{
    float resultX();
    float resultY();
    float resultZ();
    float extraValue();
}

struct BasisScaled : IMixed
{
    float3 position;
    float3x3 basis;
    float scale;

    float resultX()
    {
        float3 r = mul(basis, position) * scale;
        return r.x;
    }

    float resultY()
    {
        float3 r = mul(basis, position) * scale;
        return r.y;
    }

    float resultZ()
    {
        float3 r = mul(basis, position) * scale;
        return r.z;
    }

    float extraValue() { return scale; }
}

struct WorldOffset : IMixed
{
    float4x4 worldMatrix;
    float2 offset;

    float resultX()
    {
        float4 v = float4(1.0, 2.0, 3.0, 1.0);
        float4 r = mul(worldMatrix, v);
        return r.x + offset.x;
    }

    float resultY()
    {
        float4 v = float4(1.0, 2.0, 3.0, 1.0);
        float4 r = mul(worldMatrix, v);
        return r.y + offset.y;
    }

    float resultZ()
    {
        float4 v = float4(1.0, 2.0, 3.0, 1.0);
        float4 r = mul(worldMatrix, v);
        return r.z;
    }

    float extraValue() { return offset.x + offset.y; }
}

IMixed makeMixed(int id)
{
    if (id == 0)
    {
        BasisScaled a;
        a.position = float3(1.0, 2.0, 3.0);
        a.basis = float3x3(
            2.0, 1.0, 0.0,
            0.0, 3.0, 0.0,
            0.0, 0.0, 4.0);
        a.scale = 0.5;
        return a;
    }
    else
    {
        WorldOffset b;
        b.worldMatrix = float4x4(
            1.0, 0.0, 2.0, 10.0,
            0.0, 1.0, 0.0, 20.0,
            0.0, 3.0, 1.0, 30.0,
            0.0, 0.0, 0.0, 1.0);
        b.offset = float2(0.5, 0.5);
        return b;
    }
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // BasisScaled: [[2,1,0],[0,3,0],[0,0,4]] * (1,2,3) * 0.5 = (4,6,12)*0.5 = (2,3,6)
    var a = makeMixed(id);
    outputBuffer[0] = a.resultX();
    // CHECK: 2.0
    outputBuffer[1] = a.resultY();
    // CHECK: 3.0
    outputBuffer[2] = a.resultZ();
    // CHECK: 6.0
    outputBuffer[3] = a.extraValue();
    // CHECK: 0.5

    // WorldOffset: [[1,0,2,10],[0,1,0,20],[0,3,1,30],[0,0,0,1]] * (1,2,3,1) + offset
    var b = makeMixed(id + 1);
    outputBuffer[4] = b.resultX();
    // CHECK: 17.5
    outputBuffer[5] = b.resultY();
    // CHECK: 22.5
    outputBuffer[6] = b.resultZ();
    // CHECK: 39.0
    outputBuffer[7] = b.extraValue();
    // CHECK: 1.0
}
