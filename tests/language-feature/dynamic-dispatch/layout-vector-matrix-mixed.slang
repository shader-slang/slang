// Mixed field types in interface implementations that go through dynamic dispatch.
// Impl A uses vectors + scalar (28 bytes), Impl B uses matrix + vectors (56 bytes),
// exercising different-sized AnyValue payloads and composite packing.
// Note: a struct with float3 preceding float3x3 (e.g. float3 + float3x3 + float)
// triggers a Metal compiler failure due to alignment, so Impl A avoids that
// layout. Impl B places the matrix first (float3x3 + float3 + float2) which
// compiles correctly on Metal.

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

// 7 floats = 28 bytes (vectors + scalar)
struct VecScaled : IMixed
{
    float3 position;
    float3 direction;
    float scale;

    float resultX() { return (position.x + direction.x) * scale; }
    float resultY() { return (position.y + direction.y) * scale; }
    float resultZ() { return (position.z + direction.z) * scale; }
    float extraValue() { return scale; }
}

struct WorldOffset : IMixed
{
    float3x3 worldMatrix;
    float3 translation;
    float2 offset;

    float resultX()
    {
        float3 v = float3(1.0, 2.0, 3.0);
        float3 r = mul(worldMatrix, v) + translation;
        return r.x + offset.x;
    }

    float resultY()
    {
        float3 v = float3(1.0, 2.0, 3.0);
        float3 r = mul(worldMatrix, v) + translation;
        return r.y + offset.y;
    }

    float resultZ()
    {
        float3 v = float3(1.0, 2.0, 3.0);
        float3 r = mul(worldMatrix, v) + translation;
        return r.z;
    }

    float extraValue() { return offset.x + offset.y; }
}

IMixed makeMixed(int id)
{
    if (id == 0)
    {
        VecScaled a;
        a.position = float3(1.0, 2.0, 3.0);
        a.direction = float3(3.0, 4.0, 9.0);
        a.scale = 0.5;
        return a;
    }
    else
    {
        WorldOffset b;
        b.worldMatrix = float3x3(
            1.0, 0.0, 2.0,
            0.0, 1.0, 0.0,
            0.0, 3.0, 1.0);
        b.translation = float3(10.0, 20.0, 30.0);
        b.offset = float2(0.5, 0.5);
        return b;
    }
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // VecScaled: (pos + dir) * scale = (1+3, 2+4, 3+9) * 0.5 = (2, 3, 6)
    var a = makeMixed(id);
    outputBuffer[0] = a.resultX();
    // CHECK: 2.0
    outputBuffer[1] = a.resultY();
    // CHECK: 3.0
    outputBuffer[2] = a.resultZ();
    // CHECK: 6.0
    outputBuffer[3] = a.extraValue();
    // CHECK: 0.5

    // WorldOffset: [[1,0,2],[0,1,0],[0,3,1]] * (1,2,3) + (10,20,30) + offset
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
