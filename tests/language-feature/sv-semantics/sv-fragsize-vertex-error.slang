//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage vertex -entry main

// CHECK: error 38028

[shader("vertex")]
void main(
    int2 fragSize : SV_FragSize,
    out float4 position : SV_Position)
{
    position = float4(fragSize, 0, 1);
}


