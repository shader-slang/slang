//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage vertex -entry main

// CHECK: error 30702

[shader("vertex")]
void main(
    int layer : SV_RenderTargetArrayIndex,
    out float4 position : SV_Position)
{
    position = float4(layer, 0, 0, 1);
}


