//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage vertex -entry main

// CHECK: error 38028

[shader("vertex")]
void main(
    uint3 groupThreadID : SV_GroupThreadID,
    out float4 position : SV_Position)
{
    position = float4(groupThreadID, 1);
}


