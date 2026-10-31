//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-mtl -compute -shaderobj -output-using-type

// The host writes `Args` in natural layout (`float3` is 12 bytes, so `B`
// starts at byte 12 and `t` at byte 24), and the shader reads it through a
// constant buffer that names `ScalarDataLayout`. Reading 1..7 back shows that
// the shader and the reported offsets agree.

struct Args
{
    float3 A;
    float3 B;
    float t;
};

//TEST_INPUT:set cb = cbuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 0.0])
ConstantBuffer<Args, ScalarDataLayout> cb;

//TEST_INPUT:set output = out ubuffer(data=[0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = cb.A.x;
    output[1] = cb.A.y;
    output[2] = cb.A.z;
    output[3] = cb.B.x;
    output[4] = cb.B.y;
    output[5] = cb.B.z;
    output[6] = cb.t;
}

// BUF: 1.0
// BUF-NEXT: 2.0
// BUF-NEXT: 3.0
// BUF-NEXT: 4.0
// BUF-NEXT: 5.0
// BUF-NEXT: 6.0
// BUF-NEXT: 7.0
