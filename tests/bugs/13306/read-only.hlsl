//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -Gec -no-codegen

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
//CHECK:                 ^ left of '=' is not an l-value
//CHECK:                 ^ left of '=' is not an l-value.
    value = 1;
//CHECK:  ^ left of '=' is not an l-value
//CHECK:  ^ left of '=' is not an l-value.
}
