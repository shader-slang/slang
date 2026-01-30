//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main

// CHECK: error 30702

[shader("fragment")]
void main(
    out float4 position : SV_Position)
{
    position = float4(0, 0, 0, 1);
}


