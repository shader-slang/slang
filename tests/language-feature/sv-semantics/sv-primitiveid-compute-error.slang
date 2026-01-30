//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry main

// CHECK: error 30702

[shader("compute")]
[numthreads(1,1,1)]
void main(
    uint primID : SV_PrimitiveID)
{
}


