//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage vertex -entry main

// CHECK: error 30703

[shader("vertex")]
void main(
    int sampleIndex : SV_SampleIndex,
    out float4 position : SV_Position)
{
    position = float4(sampleIndex, 0, 0, 1);
}


