//TEST:SIMPLE_EX(filecheck=CHECK): tests/bugs/13306/read-only.hlsl -Gec -target hlsl -entry main

// We check that semantic checking rejects writes to resource parameters and buffer contents
// when their types cannot be stored in mutable globals. Reading these aliases is still allowed.
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
    // CHECK: error[E30011]
    explicitBuffer.value = 1;
    // CHECK: error[E30011]
    value = 1;
    // CHECK: error[E30011]
}
