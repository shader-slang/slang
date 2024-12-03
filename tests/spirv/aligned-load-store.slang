//TEST:SIMPLE(filecheck=CHECK): -target spirv

// CHECK: OpLoad {{.*}} Aligned 8
// CHECK: OpStore {{.*}} Aligned 16

uniform float4* data;

[numthreads(1,1,1)]
void computeMain()
{
    var v = loadAligned<8>((float2x4*)data);
    storeAligned<16>((float2x4*)data+1, v);
}