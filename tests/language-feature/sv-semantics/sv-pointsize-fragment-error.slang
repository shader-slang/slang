//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main

// CHECK: error 40015

[shader("fragment")]
float4 main(
    float pointSize : SV_PointSize)
    : SV_Target
{
    return float4(pointSize, 0, 0, 1);
}


