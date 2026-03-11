// Bare vector fields (float2, float3, float4) in interface implementations.
// Each vector type packs multiple scalar components into AnyValue storage;
// this verifies that all components survive the marshalling round-trip
// and remain usable for per-component access and vector arithmetic.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgpu -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IVecField
{
    float readX();
    float readY();
    float readZ();
    float readW();
    float dotProduct();
    float vecLength();
}

struct TwoVec3 : IVecField
{
    float3 position;
    float3 velocity;

    float readX() { return position.x; }
    float readY() { return position.y; }
    float readZ() { return position.z; }
    float readW() { return velocity.y; }
    float dotProduct() { return dot(position, velocity); }
    float vecLength() { return length(position); }
}

struct Vec4Vec2 : IVecField
{
    float4 color;
    float2 uv;

    float readX() { return color.x; }
    float readY() { return color.y; }
    float readZ() { return color.z; }
    float readW() { return color.w; }
    float dotProduct() { return dot(uv, float2(color.x, color.y)); }
    float vecLength() { return length(uv); }
}

IVecField makeVecData(int id)
{
    if (id == 0)
    {
        TwoVec3 v;
        v.position = float3(3.0, 0.0, 4.0);
        v.velocity = float3(1.0, 2.0, 0.0);
        return v;
    }
    else
    {
        Vec4Vec2 v;
        v.color = float4(1.0, 2.0, 0.0, 0.5);
        v.uv = float2(3.0, 4.0);
        return v;
    }
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // TwoVec3: position=(3,0,4), velocity=(1,2,0)
    var a = makeVecData(id);
    outputBuffer[0] = a.readX();
    // CHECK: 3.0
    outputBuffer[1] = a.readY();
    // CHECK: 0.0
    outputBuffer[2] = a.readZ();
    // CHECK: 4.0
    outputBuffer[3] = a.readW();
    // CHECK: 2.0
    outputBuffer[4] = a.dotProduct();
    // CHECK: 3.0
    outputBuffer[5] = a.vecLength();
    // CHECK: 5.0

    // Vec4Vec2: color=(1,2,0,0.5), uv=(3,4)
    var b = makeVecData(id + 1);
    outputBuffer[6] = b.readX();
    // CHECK: 1.0
    outputBuffer[7] = b.readY();
    // CHECK: 2.0
    outputBuffer[8] = b.readZ();
    // CHECK: 0.0
    outputBuffer[9] = b.readW();
    // CHECK: 0.5
    outputBuffer[10] = b.dotProduct();
    // CHECK: 11.0
    outputBuffer[11] = b.vecLength();
    // CHECK: 5.0
}
