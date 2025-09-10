// Test case to reproduce the std430 cbuffer ArrayStride issue

layout(std430) cbuffer CBlock {
    int2 bb[2];
};

struct B {
    int2 a[2];
};

ConstantBuffer<B, Std430DataLayout> c;

float4 imageMain(uint2 dispatchThreadID, int2 screenSize)
{
    return float4(0.3, 0.7, 0.55, 1.0
    + bb[1].y
    + c.a[0].x
    );
}