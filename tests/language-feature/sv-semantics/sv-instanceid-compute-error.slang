//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry main

// CHECK: error 40015

[shader("compute")]
[numthreads(1,1,1)]
void main(
    uint instanceID : SV_InstanceID)
{
}


