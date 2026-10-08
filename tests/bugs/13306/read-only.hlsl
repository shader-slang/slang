//TEST:SIMPLE_EX(filecheck=CHECK): tests/bugs/13306/read-only.hlsl -Gec -target hlsl -entry main

// Compatibility permits rebinding a direct resource through its mutable shadow. Explicit parameter
// groups and legacy-buffer contents remain read-only aliases, so their writes must be rejected.
Texture2D<float4> gTex;
Texture2D<float4> other;
struct Data { uint value; };
ConstantBuffer<Data> explicitBuffer;
cbuffer Mixed
{
    Texture2D<float4> texture;
    uint value;
};

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    gTex = other;
    explicitBuffer.value = 1;
    // CHECK: error[E30011]
    value = 1;
    // CHECK: error[E30011]
}
