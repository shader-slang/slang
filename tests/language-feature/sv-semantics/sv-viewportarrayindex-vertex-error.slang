//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage vertex -entry main

// CHECK: error 38028

[shader("vertex")]
void main(
    int viewport : SV_ViewportArrayIndex,
    out float4 position : SV_Position)
{
    position = float4(viewport, 0, 0, 1);
}


