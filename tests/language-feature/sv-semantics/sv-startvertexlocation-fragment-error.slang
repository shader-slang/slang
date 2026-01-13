//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main

// CHECK: error 40015

[shader("fragment")]
float4 main(
    uint baseVertex : SV_StartVertexLocation)
    : SV_Target
{
    return float4(baseVertex, 0, 0, 1);
}


