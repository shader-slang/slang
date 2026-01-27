//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage vertex -entry main

// CHECK: error 30703

[shader("vertex")]
void main(
    bool isFront : SV_IsFrontFace,
    out float4 position : SV_Position)
{
    position = float4(isFront ? 1.0 : 0.0, 0, 0, 1);
}


