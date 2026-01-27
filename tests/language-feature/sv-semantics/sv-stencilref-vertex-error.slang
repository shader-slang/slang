//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage vertex -entry main

// CHECK: error 30703

[shader("vertex")]
void main(
    out int stencilRef : SV_StencilRef,
    out float4 position : SV_Position)
{
    stencilRef = 0;
    position = float4(0, 0, 0, 1);
}


