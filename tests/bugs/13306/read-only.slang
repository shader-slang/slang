//TEST:SIMPLE_EX(filecheck=CHECK): -lang hlsl tests/bugs/13306/read-only.slang -Gec -target hlsl -entry main

// Unsupported parameter copies remain aliases. We reject writes to the input rather than
// accepting the assignment and silently discarding it during lowering.
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
