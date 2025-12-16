//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry main

// CHECK: error 38028

[shader("compute")]
[numthreads(1,1,1)]
void main(
    uint vertexID : SV_VertexID)
{
}


