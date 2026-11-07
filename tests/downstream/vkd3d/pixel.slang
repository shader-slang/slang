//TEST:SIMPLE(filecheck=CHECK):-O1 -target dxbc-asm -profile ps_5_0 -entry pixelMain
//CHECK: ps_5_0
//CHECK: sample
//CHECK: ret

Texture2D<float4> image;
SamplerState imageSampler;

[shader("fragment")]
float4 pixelMain(float2 uv : TEXCOORD) : SV_Target
{
    return image.Sample(imageSampler, uv);
}
